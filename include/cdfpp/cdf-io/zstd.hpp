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
#include "cdfpp/no_init_vector.hpp"
#include <cstddef>
#include <vector>
#include <zstd.h>


namespace cdf::io::zstd
{
namespace _internal
{

    template <typename T>
    CDF_WARN_UNUSED_RESULT std::size_t impl_inflate(
        const T& input, char* output, const std::size_t output_size)
    {
        const auto ret = ZSTD_decompress(output, output_size, input.data(), std::size(input));

        if (ret != ZSTD_isError(ret))
            return ret;
        else
            return 0;
    }

    template <typename T>
    CDF_WARN_UNUSED_RESULT no_init_vector<char> impl_deflate(const T& input)
    {
        const auto input_bytes = std::size(input) * sizeof(decltype(*input.data()));
        no_init_vector<char> result(ZSTD_compressBound(input_bytes));
        const auto ret = ZSTD_compress(result.data(), result.size(),
            reinterpret_cast<const char* const>(input.data()), input_bytes, 3);
        if (ret != ZSTD_isError(ret))
        {
            result.resize(ret);
            result.shrink_to_fit();
            return result;
        }
        else
            return {};
    }
}

template <typename T>
std::size_t inflate(const T& input, char* output, const std::size_t output_size)
{
    using namespace _internal;
    return impl_inflate(input, output, output_size);
}

template <typename T>
no_init_vector<char> deflate(const T& input)
{
    using namespace _internal;
    return impl_deflate(input);
}
}
