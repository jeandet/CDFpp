#pragma once
/*------------------------------------------------------------------------------
-- This file is a part of the CDFpp library
-- Copyright (C) 2024, Plasma Physics Laboratory - CNRS
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
-------------------------------------------------------------------------------*/
/*-- Author : Alexis Jeandet
-- Mail : alexis.jeandet@member.fsf.org
----------------------------------------------------------------------------*/
#include "../cdf-debug.hpp"
#include "../cdf-enums.hpp"
#include "cdfpp/no_init_vector.hpp"
#include "endianness.hpp"
#include "zstd.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <vector>


namespace cdf::io::experimental_compression
{

namespace delta_zstd
{
    namespace _internal
    {
        template <std::size_t size>
        auto _signed_int()
        {
            if constexpr (size == 1)
                return std::int8_t {};
            else if constexpr (size == 2)
                return std::int16_t {};
            else if constexpr (size == 4)
                return std::int32_t {};
            else if constexpr (size == 8)
                return std::int64_t {};
        }

        template <std::size_t size>
        using signed_int_t = decltype(_signed_int<size>());


        template <typename T>
        std::size_t impl_delta_inflate(T* const output, const std::size_t output_size)
        {
            std::partial_sum(output, output + (output_size / sizeof(T)), output);
            return output_size;
        }

        template <typename T>
        std::size_t impl_inflate(
            const T& input, char* output, const std::size_t output_size, const CDF_Types cdf_type)
        {
            zstd::inflate(input, output, output_size);
            switch (cdf_type_size(cdf_type))
            {
                case 1:
                    return impl_delta_inflate(
                        reinterpret_cast<uint8_t* const>(output), output_size);
                    break;
                case 2:
                    return impl_delta_inflate(
                        reinterpret_cast<uint16_t* const>(output), output_size);
                    break;
                case 4:
                    return impl_delta_inflate(
                        reinterpret_cast<uint32_t* const>(output), output_size);
                    break;
                case 8:
                    return impl_delta_inflate(
                        reinterpret_cast<uint64_t* const>(output), output_size);
                    break;
                default:
                    break;
            }
            throw std::runtime_error("Invalid CDF type");
        }


        template <typename T>
        no_init_vector<char> impl_deflate(const T* const input, std::size_t length)
        {
            std::vector<T> deltas(length);
            deltas[0] = input[0];
            std::transform(std::next(input), input + length, input, std::next(std::begin(deltas)),
                [](const auto& in1, const auto& in2) { return in1 - in2; });
            return zstd::deflate(deltas);
        }

        template <typename T>
        no_init_vector<char> impl_deflate(const T& input, const CDF_Types cdf_type)
        {
            const auto w = cdf_type_size(cdf_type);
            const auto count = std::size(input) * sizeof(decltype(*input.data())) / w;
            switch (w)
            {
                case 1:
                    return impl_deflate(
                        reinterpret_cast<const uint8_t* const>(input.data()), count);
                    break;
                case 2:
                    return impl_deflate(
                        reinterpret_cast<const uint16_t* const>(input.data()), count);
                    break;
                case 4:
                    return impl_deflate(
                        reinterpret_cast<const uint32_t* const>(input.data()), count);
                    break;
                case 8:
                    return impl_deflate(
                        reinterpret_cast<const uint64_t* const>(input.data()), count);
                    break;
                default:
                    throw std::runtime_error("Invalid CDF type");
            }
        }
    }
    template <typename T>
    std::size_t inflate(
        const T& input, char* output, const std::size_t output_size, const CDF_Types cdf_type)
    {
        using namespace _internal;
        return impl_inflate(input, output, output_size, cdf_type);
    }

    template <typename T>
    no_init_vector<char> deflate(const T& input, const CDF_Types cdf_type)
    {
        if (!is_integral(cdf_type))
            throw std::runtime_error("Invalid CDF type");
        using namespace _internal;
        return impl_deflate(input, cdf_type);
    }
}

namespace float_zstd
{
    namespace _internal
    {

        struct float32_fields
        {
            uint32_t mantissa : 23;
            uint32_t exponent : 8;
            uint32_t sign : 1;
        };

        struct float64_fields
        {
            uint64_t mantissa : 52;
            uint64_t exponent : 11;
            uint64_t sign : 1;
        };

        struct offsets_t
        {
            offsets_t() = default;
            offsets_t(const uint64_t exponent_offset, const uint64_t mantissa_offset)
                    : exponent_offset(exponent_offset), mantissa_offset(mantissa_offset)
            {
            }
            offsets_t(const char* const input)
            {
                exponent_offset
                    = cdf::endianness::decode<endianness::big_endian_t, uint64_t>(input);
                mantissa_offset = cdf::endianness::decode<endianness::big_endian_t, uint64_t>(
                    input + sizeof(uint64_t));
            }
            inline std::size_t write(char* output) const
            {
                auto v
                    = cdf::endianness::decode<endianness::big_endian_t, uint64_t>(&exponent_offset);
                std::memcpy(output, &v, sizeof(v));
                v = cdf::endianness::decode<endianness::big_endian_t, uint64_t>(&mantissa_offset);
                std::memcpy(output + sizeof(uint64_t), &v, sizeof(v));
                return sizeof(uint64_t) * 2;
            }

            uint64_t exponent_offset;
            uint64_t mantissa_offset;
        };

        template <typename T>
        using exponent_t = std::conditional_t<std::is_same_v<T, float>, uint8_t, uint16_t>;

        template <typename T>
        using mantissa_t = std::conditional_t<std::is_same_v<T, float>, uint32_t, uint64_t>;

        template <typename T>
        union split_float
        {
            std::conditional_t<std::is_same_v<T, float>, float32_fields, float64_fields> fields;
            T value;
        };

        template <typename T, typename float_t>
        std::size_t impl_inflate(const T& input, char* output, const std::size_t output_size,
            const std::size_t record_size)
        {
            const offsets_t offsets { input.data() };
            const auto floats_count = output_size / sizeof(float_t);
            std::vector<char> sign(floats_count);
            std::vector<exponent_t<float_t>> exponents(floats_count);
            std::vector<mantissa_t<float_t>> mantissas(floats_count);

            zstd::inflate(std::string_view { input.data() + sizeof(offsets),
                              offsets.exponent_offset - sizeof(offsets) },
                sign.data(), std::size(sign));
            delta_zstd::inflate(std::string_view { input.data() + offsets.exponent_offset,
                                    offsets.mantissa_offset - offsets.exponent_offset },
                reinterpret_cast<char* const>(exponents.data()),
                std::size(exponents) * sizeof(exponents[0]),
                sizeof(exponent_t<float_t>) == 1 ? CDF_Types::CDF_INT1 : CDF_Types::CDF_INT2);
            delta_zstd::inflate(std::string_view { input.data() + offsets.mantissa_offset,
                                    std::size(input) - offsets.mantissa_offset },
                reinterpret_cast<char* const>(mantissas.data()),
                std::size(mantissas) * sizeof(mantissas[0]),
                sizeof(mantissa_t<float_t>) == 4 ? CDF_Types::CDF_INT4 : CDF_Types::CDF_INT8);

            const auto record_count = floats_count / record_size;
            assert(record_count * record_size == floats_count);
            {
                auto linear_index = 0;
                for (std::size_t i = 0; i < record_size; i++)
                {
                    auto offset = i;
                    for (std::size_t r = 0; r < record_count; r++)
                    {
                        split_float<float_t> f;
                        f.fields.sign = sign[offset];
                        f.fields.exponent = exponents[offset];
                        f.fields.mantissa = mantissas[offset];
                        reinterpret_cast<float_t* const>(output)[linear_index] = f.value;
                        linear_index++;
                        offset += record_size;
                    }
                }
            }
            return output_size;
        }

        template <typename T>
        std::size_t impl_inflate(const T& input, char* output, const std::size_t output_size,
            const CDF_Types cdf_type, const std::size_t record_size)
        {
            switch (cdf_type)
            {
                case CDF_Types::CDF_FLOAT:
                case CDF_Types::CDF_REAL4:
                    return impl_inflate<T, float>(input, output, output_size, record_size);
                    break;
                case CDF_Types::CDF_DOUBLE:
                case CDF_Types::CDF_REAL8:
                    return impl_inflate<T, double>(input, output, output_size, record_size);
                    break;
                default:
                    throw std::runtime_error("Invalid CDF type");
                    break;
            }
        }

        template <typename exponent_t, typename mantissa_t>
        no_init_vector<char> layout(const no_init_vector<char>& sign_deflated,
            const exponent_t& exponents_deflated, const mantissa_t& mantissas_deflated)
        {
            no_init_vector<char> result(sizeof(offsets_t) + std::size(sign_deflated)
                + std::size(exponents_deflated) + std::size(mantissas_deflated));

            const offsets_t offsets { sizeof(offsets_t) + std::size(sign_deflated),
                sizeof(offsets_t) + std::size(sign_deflated) + std::size(exponents_deflated) };
            const auto cursor = offsets.write(result.data());

            std::memcpy(result.data() + cursor, sign_deflated.data(), std::size(sign_deflated));

            std::memcpy(result.data() + offsets.exponent_offset, exponents_deflated.data(),
                std::size(exponents_deflated));

            std::memcpy(result.data() + offsets.mantissa_offset, mantissas_deflated.data(),
                std::size(mantissas_deflated));

            return result;
        }

        template <typename T>
        no_init_vector<char> impl_deflate(
            const T* const input, std::size_t length, const std::size_t record_size)
        {
            std::vector<char> sign(length);
            std::vector<exponent_t<T>> exponents(length);
            std::vector<mantissa_t<T>> mantissas(length);
            const auto record_count = length / record_size;
            assert(record_count * record_size == length);
            {
                auto linear_index = 0;
                for (std::size_t i = 0; i < record_size; i++)
                {
                    auto offset = i;
                    for (std::size_t r = 0; r < record_count; r++)
                    {
                        split_float<T> f;
                        f.value = input[offset];
                        sign[linear_index] = f.fields.sign;
                        exponents[linear_index] = f.fields.exponent;
                        mantissas[linear_index] = f.fields.mantissa;
                        linear_index++;
                        offset += record_size;
                    }
                }
            }
            auto sign_deflated = zstd::deflate(sign);
            sign.clear();
            sign.shrink_to_fit();
            auto exponents_deflated = delta_zstd::deflate(
                exponents, sizeof(exponent_t<T>) == 1 ? CDF_Types::CDF_INT1 : CDF_Types::CDF_INT2);
            exponents.clear();
            exponents.shrink_to_fit();
            auto mantissas_deflated = delta_zstd::deflate(
                mantissas, sizeof(mantissa_t<T>) == 4 ? CDF_Types::CDF_INT4 : CDF_Types::CDF_INT8);
            mantissas.clear();
            mantissas.shrink_to_fit();
            no_init_vector<char> result(sizeof(offsets_t) + std::size(sign_deflated)
                + std::size(exponents_deflated) + std::size(mantissas_deflated));

            return layout(sign_deflated, exponents_deflated, mantissas_deflated);
        }

        template <typename T>
        no_init_vector<char> impl_deflate(
            const T& input, const CDF_Types cdf_type, const std::size_t record_size)
        {
            const auto w = cdf_type_size(cdf_type);
            const auto count = std::size(input) * sizeof(decltype(*input.data())) / w;
            switch (w)
            {
                case 4:
                    return impl_deflate(
                        reinterpret_cast<const float* const>(input.data()), count, record_size);
                    break;
                case 8:
                    return impl_deflate(
                        reinterpret_cast<const double* const>(input.data()), count, record_size);
                    break;
                default:
                    throw std::runtime_error("Invalid CDF type");
                    break;
            }
        }

    }

    template <typename T>
    std::size_t inflate(const T& input, char* output, const std::size_t output_size,
        const CDF_Types cdf_type, const std::size_t record_size = 1)
    {
        if (!is_floating_point(cdf_type))
            throw std::runtime_error("Invalid CDF type");
        using namespace _internal;
        return impl_inflate(input, output, output_size, cdf_type, record_size);
    }

    template <typename T>
    no_init_vector<char> deflate(
        const T& input, const CDF_Types cdf_type, const std::size_t record_size = 1)
    {
        if (!is_floating_point(cdf_type))
            throw std::runtime_error("Invalid CDF type");
        using namespace _internal;
        return impl_deflate(input, cdf_type, record_size);
    }
}

}
