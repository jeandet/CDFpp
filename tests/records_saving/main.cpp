#include <algorithm>
#include <optional>
#include <stdint.h>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>


#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>


#include "cdfpp/cdf-io/saving/records-saving.hpp"
#include <cpp_utils/serde/serde.hpp>

using cpp_utils::serde::bounded_string;
using cpp_utils::serde::dynamic_array;

SCENARIO("record loading", "[CDF]")
{
    GIVEN("a simple two char fields record")
    {
        struct two_chars
        {
            char a;
            char b;
        };
        two_chars s { 'a', 'b' };
        THEN("we can load it from a buffer")
        {
            REQUIRE(cdf::io::record_size(s)==2);
            static_assert(cdf::io::record_size(s)==2);
        }
    }
    GIVEN("a more complex record")
    {
        struct complex_record
        {
            char a;
            double b;
            uint32_t c;
            uint64_t d;
        };
        THEN("we can load it from a buffer")
        {
            complex_record s { 0, 0., 0, 0 };
            REQUIRE(cdf::io::record_size(s)==21);
            static_assert(cdf::io::record_size(s)==21);
        }
    }
    GIVEN("a record with nested record")
    {
        struct inner_record
        {
            uint16_t a;
            uint16_t b;
        };
        struct outer_record
        {
            char a;
            inner_record b;
            char c;
        };
        THEN("we can load it from a buffer")
        {
            outer_record s { 0, { 0, 0 }, 0 };
            REQUIRE(cdf::io::record_size(s)==6);
            static_assert(cdf::io::record_size(s)==6);
        }
    }
    GIVEN("a record with string fields")
    {
        struct record_with_string
        {
            char a;
            double b;
            bounded_string<8> c;
            uint64_t d;
        };
        THEN("we can load it from a buffer")
        {
            record_with_string s { 0, 0., { "" }, 0 };

            REQUIRE(cdf::io::record_size(s)==25);
            static_assert(cdf::io::record_size(s)==25);
        }
    }
    GIVEN("a record with table fields")
    {
        struct record_table_fields
        {
            char a;
            double b;
            dynamic_array<0, uint16_t> c;
            uint64_t d;
            dynamic_array<1, uint32_t> e;

            std::size_t field_size(const dynamic_array<0, uint16_t>&) const
            {
                return this->a;
            }
            std::size_t field_size(const dynamic_array<1, uint32_t>&) const
            {
                return 2;
            }
        };
        THEN("we can load it from a buffer")
        {
            // record_size (unlike load) sums each dynamic_array's OWN current
            // .size(), not field_size() — matching how CDFpp actually saves:
            // create_records.hpp always resizes an array to its intended element
            // count before update_size()/save_record() ever runs, so field_size()
            // only needs to be consulted on the load path (deserialize's
            // resolve_field_size). Populate `e` to the 2 elements field_size()
            // would have implied, matching real usage.
            record_table_fields s{0,0.,{},0,{}};
            s.e.resize(2);
            REQUIRE(cdf::io::record_size(s)==25);
        }
    }
    GIVEN("a true CDF record")
    {

        THEN("we can load it from a buffer")
        {
            io::cdf_CDR_t<io::v3x_tag> s{};
            static_assert(io::is_cdf_DR_header_v<decltype(s.header)>);
            static_assert(cdf::io::record_size(s)==312);
        }
    }
}
