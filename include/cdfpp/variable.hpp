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
#include "attribute.hpp"
#include "cdf-data.hpp"
#include "cdf-enums.hpp"
#include "cdf-majority-swap.hpp"
#include <cstdint>
#include <optional>
#include "cdf-map.hpp"
#include <vector>

namespace cdf
{
struct Variable
{
    using var_data_t = data_t;
    using shape_t = std::vector<uint32_t>;
    cdf_map<std::string, Attribute> attributes;
    Variable() = default;
    Variable(Variable&&) = default;
    Variable(const Variable&) = default;
    Variable& operator=(const Variable&) = default;
    Variable& operator=(Variable&&) = default;
    Variable(const std::string& name, std::size_t number, var_data_t&& data, shape_t&& shape,
        cdf_majority majority)
            : p_name { name }
            , p_number { number }
            , p_data { std::move(data) }
            , p_shape { shape }
            , p_majority { majority }
    {
        if (this->majority() == cdf_majority::column)
        {
            majority::swap(_data(), p_shape);
        }
    }

    Variable(const std::string& name, std::size_t number, lazy_data&& data, shape_t&& shape,
        cdf_majority majority)
            : p_name { name }
            , p_number { number }
            , p_data { std::move(data) }
            , p_shape { shape }
            , p_majority { majority }
    {
    }


    template <CDF_Types type>
    decltype(auto) get()
    {
        return _data().get<type>();
    }

    template <CDF_Types type>
    decltype(auto) get() const
    {
        return _data().get<type>();
    }

    template <typename type>
    decltype(auto) get()
    {
        return _data().get<type>();
    }

    template <typename type>
    decltype(auto) get() const
    {
        return _data().get<type>();
    }

    const std::string& name() const { return p_name; }

    const shape_t& shape() const { return p_shape; }
    std::size_t len() const { return p_shape[0]; }

    void set_data(const data_t& data, const shape_t& shape)
    {
        p_data = data;
        p_shape = shape;
    }

    void set_data(data_t&& data, shape_t&& shape)
    {
        p_data = std::move(data);
        p_shape = std::move(shape);
    }

    std::optional<std::size_t> number() { return p_number; }

    CDF_Types type() const
    {
        if (std::holds_alternative<var_data_t>(p_data))
            return std::get<var_data_t>(p_data).type();
        return std::get<lazy_data>(p_data).type();
    }

    inline bool values_loaded()const
    {
        return not std::holds_alternative<lazy_data>(p_data);
    }

    inline void load_values() const
    {
        if (not values_loaded())
        {
            p_data = std::get<lazy_data>(p_data).load();
            auto& data = std::get<data_t>(p_data);
            if (this->majority() == cdf_majority::column)
            {
                majority::swap(data, p_shape);
            }
        }
    }

    cdf_majority majority() const { return p_majority; }

    template <typename... Ts>
    friend auto visit(Variable& var, Ts... lambdas);

private:
    var_data_t& _data()
    {
        load_values();
        return std::get<var_data_t>(p_data);
    }
    const var_data_t& _data() const
    {
        load_values();
        return std::get<var_data_t>(p_data);
    }

    std::string p_name;
    std::optional<std::size_t> p_number;
    mutable std::variant<lazy_data, var_data_t> p_data;
    shape_t p_shape;
    cdf_majority p_majority;
};

template <typename... Ts>
auto visit(Variable& var, Ts... lambdas)
{
    return visit(var.p_data, lambdas...);
}
} // namespace cdf
