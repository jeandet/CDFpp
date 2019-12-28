#pragma once
/*------------------------------------------------------------------------------
-- This file is a part of the CDFpp library
-- Copyright (C) 2019, Plasma Physics Laboratory - CNRS
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
#include "cdf-io-desc-records.hpp"
#include <type_traits>

namespace cdf::io::common
{
bool is_v3x(const magic_numbers_t& magic)
{
    return ((magic.first >> 12) & 0xff) >= 0x30;
}

bool is_cdf(const magic_numbers_t& magic_numbers) noexcept
{
    return (magic_numbers.first & 0xfff00000) == 0xCDF00000
        && (magic_numbers.second == 0xCCCC0001 || magic_numbers.second == 0x0000FFFF);
}

template <typename context_t>
bool is_cdf(const context_t& context) noexcept
{
    return is_cdf(context.magic);
}

bool is_compressed(const magic_numbers_t& magic_numbers) noexcept
{
    return magic_numbers.second == 0xCCCC0001;
}

template <typename value_t, typename stream_t>
struct blk_iterator
{
    using iterator_category = std::forward_iterator_tag;
    using value_type = value_t;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = value_type&;

    std::size_t offset;
    value_t block;
    stream_t& stream;
    std::function<std::size_t(value_t&)> next;

    blk_iterator(std::size_t offset, stream_t& stream, std::function<std::size_t(value_t&)>&& next)
            : offset { offset }, block { stream }, stream { stream }, next { std::move(next) }
    {
        if (offset != 0)
            block.load(offset);
    }

    auto operator==(const blk_iterator& other) { return other.offset == offset; }

    bool operator!=(const blk_iterator& other) { return not(*this == other); }

    blk_iterator& operator+(int n)
    {
        step_forward(n);
        return *this;
    }

    blk_iterator& operator+=(int n)
    {
        step_forward(n);
        return *this;
    }

    blk_iterator& operator++(int n)
    {
        step_forward(n);
        return *this;
    }

    blk_iterator& operator++()
    {
        step_forward();
        return *this;
    }

    void step_forward(int n = 1)
    {
        while (n > 0)
        {
            n--;
            offset = next(block);
            if (offset != 0)
            {
                block.load(offset);
            }
        }
    }

    const value_type* operator->() const { return &block; }
    const value_type& operator*() const { return block; }
    value_type* operator->() { return &block; }
    value_type& operator*() { return block; }
};

template <typename version_t, typename stream_t>
auto begin_ADR(const cdf_GDR_t<version_t, stream_t>& gdr)
{
    return blk_iterator<cdf_ADR_t<version_t, stream_t>, stream_t> { gdr.ADRhead.value, gdr.p_stream,
        [](const auto& ADR) { return ADR.ADRnext.value; } };
}

template <typename version_t, typename stream_t>
auto end_ADR(const cdf_GDR_t<version_t, stream_t>& gdr)
{
    return blk_iterator<cdf_ADR_t<version_t, stream_t>, stream_t> { 0, gdr.p_stream,
        [](const auto& ADR) { return 0; } };
}

template <cdf_r_z type, typename version_t, typename stream_t>
auto begin_AEDR(const cdf_ADR_t<version_t, stream_t>& adr)
{
    if constexpr (type == cdf_r_z::r)
    {
        return blk_iterator<cdf_AEDR_t<version_t, stream_t>, stream_t> { adr.AgrEDRhead.value,
            adr.p_stream, [](const auto& AEDR) { return AEDR.AEDRnext.value; } };
    }
    else if constexpr (type == cdf_r_z::z)
    {
        return blk_iterator<cdf_AEDR_t<version_t, stream_t>, stream_t> { adr.AzEDRhead.value,
            adr.p_stream, [](const auto& AEDR) { return AEDR.AEDRnext.value; } };
    }
}

template <cdf_r_z type, typename version_t, typename stream_t>
auto end_AEDR(const cdf_ADR_t<version_t, stream_t>& adr)
{
    return blk_iterator<cdf_AEDR_t<version_t, stream_t>, stream_t> { 0, adr.p_stream,
        [](const auto& ADR) { return 0; } };
}

template <cdf_r_z type, typename version_t, typename stream_t>
auto begin_VDR(const cdf_GDR_t<version_t, stream_t>& gdr)
{
    if constexpr (type == cdf_r_z::r)
    {
        return blk_iterator<cdf_VDR_t<version_t, stream_t>, stream_t> { gdr.rVDRhead.value,
            gdr.p_stream, [](const auto& VDR) { return VDR.VDRnext.value; } };
    }
    else if constexpr (type == cdf_r_z::z)
    {
        return blk_iterator<cdf_VDR_t<version_t, stream_t>, stream_t> { gdr.zVDRhead.value,
            gdr.p_stream, [](const auto& VDR) { return VDR.VDRnext.value; } };
    }
}

template <cdf_r_z type, typename version_t, typename stream_t>
auto end_VDR(const cdf_GDR_t<version_t, stream_t>& gdr)
{
    return blk_iterator<cdf_VDR_t<version_t, stream_t>, stream_t> { 0, gdr.p_stream,
        [](const auto& VDR) { return 0; } };
}

}
