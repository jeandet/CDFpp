#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>


#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>


#include <chrono>


#include "cdfpp/attribute.hpp"
#include "cdfpp/cdf-debug.hpp"
#include "cdfpp/cdf-file.hpp"
#include "cdfpp/cdf-io/cdf-io.hpp"
#include "cdfpp/chrono/cdf-chrono.hpp"
#include "cdfpp/variable.hpp"

#include "tests_config.hpp"


template <typename T>
struct cos_gen
{
    const T step;
    cos_gen(T step) : step { step } { }
    no_init_vector<T> operator()(std::size_t size)
    {
        no_init_vector<T> values(size);
        std::generate(std::begin(values), std::end(values),
            [i = T(0.), step = step]() mutable
            {
                auto v = std::cos(i);
                i += step;
                return v;
            });
        return values;
    }
};

template <typename T>
struct ones
{
    no_init_vector<T> operator()(std::size_t size)
    {
        no_init_vector<T> values(size);
        std::generate(std::begin(values), std::end(values), []() mutable { return T(1); });
        return values;
    }
};

template <typename T>
struct zeros
{
    no_init_vector<T> operator()(std::size_t size)
    {
        no_init_vector<T> values(size);
        std::generate(std::begin(values), std::end(values), []() mutable { return T(0); });
        return values;
    }
};


SCENARIO("Saving a cdf file", "[CDF]")
{
    auto cdf_path = std::tmpnam(nullptr);

    {
        CDF cdf_obj;
        cdf_obj.attributes.emplace("some global attr",
            cdf::Attribute { "some global attr",
                { data_t { no_init_vector<double> { 1., 2., 3. }, CDF_Types::CDF_DOUBLE } } });
        cdf_obj.attributes.emplace("another global attr",
            cdf::Attribute { "another global attr",
                { data_t {
                    no_init_vector<char> { 'h', 'e', 'l', 'l', 'o' }, CDF_Types::CDF_CHAR } } });

        cdf_obj.variables.emplace("var1",
            Variable { "var1", 0, data_t { zeros<float> {}(100), CDF_Types::CDF_FLOAT }, { 100 } });
        cdf_obj.variables["var1"].set_compression_type(cdf_compression_type::gzip_compression);
        REQUIRE(cdf::io::save(cdf_obj, cdf_path));
    }
    {
        auto cdf_obj = cdf::io::load(cdf_path);
        REQUIRE(cdf_obj->attributes.count("some global attr"));
        REQUIRE(cdf_obj->attributes.count("another global attr"));
        REQUIRE(cdf_obj->variables.count("var1"));
    }
}

namespace
{
// Loads a real, on-disk fixture, saves it to an in-memory buffer, then reloads
// that buffer. Comparing the reloaded CDF against the original (rather than the
// original file's own bytes) tolerates legitimate layout differences (e.g.
// update_size choices) while still catching any bug that corrupts record
// headers, values, or metadata on the way through save()+load().
std::pair<CDF, CDF> saved_and_reloaded(const std::string& fixture_name)
{
    auto path = std::string(DATA_PATH) + "/" + fixture_name;
    auto original = cdf::io::load(path);
    REQUIRE(original != std::nullopt);

    auto saved = cdf::io::save(*original);
    REQUIRE(std::size(saved) > 0);

    // Named lvalue, loaded eagerly (lazy_load=false): cdf::io::load's std::vector<char>
    // overload wraps a non-owning view over this buffer, so the buffer must outlive
    // any lazy/deferred read of the reloaded CDF's variable values. Passing it as a
    // temporary and/or with lazy_load=true would leave the reloaded CDF holding a
    // dangling view the moment the buffer goes away.
    std::vector<char> buffer(std::cbegin(saved), std::cend(saved));
    auto reloaded = cdf::io::load(buffer, true, false);
    REQUIRE(reloaded != std::nullopt);

    return { *original, *reloaded };
}
}

SCENARIO("Round-tripping real CDF fixtures through save and reload", "[CDF]")
{
    GIVEN("a real, uncompressed CDF fixture")
    {
        THEN("saving then reloading it yields a structurally and value-equal CDF")
        {
            auto [original, reloaded] = saved_and_reloaded("a_cdf.cdf");
            REQUIRE(reloaded == original);
        }
    }
    GIVEN("a real, GZIP-compressed CDF fixture")
    {
        THEN("saving then reloading it yields a structurally and value-equal CDF")
        {
            auto [original, reloaded] = saved_and_reloaded("a_compressed_cdf.cdf");
            REQUIRE(reloaded == original);
        }
    }
    GIVEN("a real, RLE-compressed CDF fixture")
    {
        THEN("saving then reloading it yields a structurally and value-equal CDF")
        {
            auto [original, reloaded] = saved_and_reloaded("a_rle_compressed_cdf.cdf");
            REQUIRE(reloaded == original);
        }
    }
    GIVEN("a real CDF fixture with per-variable (CVVR) compressed variables")
    {
        THEN("saving then reloading it yields a structurally and value-equal CDF")
        {
            auto [original, reloaded] = saved_and_reloaded("a_cdf_with_compressed_vars.cdf");
            REQUIRE(reloaded == original);
        }
    }
}
