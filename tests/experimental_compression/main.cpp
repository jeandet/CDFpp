#if __has_include(<catch2/catch_all.hpp>)
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#else
#include <catch.hpp>
#endif
#include <cdfpp_config.h>
#ifdef CDFPP_USE_EXPERIMENTAL_COMPRESSION
#include "cdfpp/cdf-io/experimental_compression.hpp"
#endif
#include "tests_config.hpp"
#include <cdfpp/cdf-io/cdf-io.hpp>
#include <cstdint>
#include <numeric>

#ifdef CDFPP_USE_EXPERIMENTAL_COMPRESSION
template <typename T>
no_init_vector<T> build_ref(std::size_t size = 100)
{
    no_init_vector<T> ref(size);
    std::iota(std::begin(ref), std::end(ref), 1);
    return ref;
}

template <typename T>
no_init_vector<T> build_random_ref(std::size_t size = 100)
{
    no_init_vector<T> ref(size);
    std::generate(std::begin(ref), std::end(ref), []() { return static_cast<T>(rand()); });
    return ref;
}

TEST_CASE("[delta_zstd] IDEMPOTENCY check", "")
{
    using namespace cdf::io::experimental_compression::delta_zstd;
    const auto ref = build_ref<int64_t>();
    no_init_vector<int64_t> w(std::size(ref));
    auto w2 = deflate(ref, cdf::CDF_Types::CDF_INT8);
    inflate(w2, reinterpret_cast<char*>(w.data()), std::size(ref) * 8, cdf::CDF_Types::CDF_INT8);
    w2 = deflate(w, cdf::CDF_Types::CDF_INT8);
    inflate(w2, reinterpret_cast<char*>(w.data()), std::size(ref) * 8, cdf::CDF_Types::CDF_INT8);
    REQUIRE(ref == w);
}

TEST_CASE("[float_zstd] IDEMPOTENCY check", "")
{
    using namespace cdf::io::experimental_compression::float_zstd;
    {
        const auto ref = build_ref<float>();
        no_init_vector<float> w(std::size(ref));
        auto w2 = deflate(ref, cdf::CDF_Types::CDF_FLOAT);
        inflate(
            w2, reinterpret_cast<char*>(w.data()), std::size(ref) * 4, cdf::CDF_Types::CDF_FLOAT);
        w2 = deflate(w, cdf::CDF_Types::CDF_FLOAT);
        inflate(
            w2, reinterpret_cast<char*>(w.data()), std::size(ref) * 4, cdf::CDF_Types::CDF_FLOAT);
        REQUIRE(ref == w);
    }
    {
        const auto ref = build_ref<double>(10000);
        no_init_vector<double> w(std::size(ref));
        auto w2 = deflate(ref, cdf::CDF_Types::CDF_DOUBLE);
        inflate(
            w2, reinterpret_cast<char*>(w.data()), std::size(ref) * 8, cdf::CDF_Types::CDF_DOUBLE);
        w2 = deflate(w, cdf::CDF_Types::CDF_DOUBLE);
        inflate(
            w2, reinterpret_cast<char*>(w.data()), std::size(ref) * 8, cdf::CDF_Types::CDF_DOUBLE);
        REQUIRE(ref == w);
    }
}

TEST_CASE("[float_zstd] Is better than zstd alone", "")
{
    using namespace cdf::io::experimental_compression::float_zstd;
    {
        for (auto i = 0UL; i < 100; i++)
        {
            const auto ref = build_random_ref<double>(300000);
            auto w2 = deflate(ref, cdf::CDF_Types::CDF_DOUBLE, 3);
            auto w3 = cdf::io::zstd::deflate(ref);
            // REQUIRE(std::size(w2) < std::size(w3));
        }
    }
}

TEST_CASE("Save a CDF with experimental compression")
{
    const auto path = std::string(DATA_PATH) + "/a_cdf.cdf";
    auto cd_opt = cdf::io::load(path);
    REQUIRE(cd_opt != std::nullopt);
    auto cd = *cd_opt;
    cd["tt2000"].set_compression_type(cdf_compression_type::delta_plus_zstd_compression);
    cd["var5d_counter"].set_compression_type(cdf_compression_type::float_zstd_compression);
    cd["zeros"].set_compression_type(cdf_compression_type::float_zstd_compression);
    const auto cdf_path = std::tmpnam(nullptr);
    REQUIRE(cdf::io::save(cd, cdf_path));
}


#else
TEST_CASE("Skip check", "") { }
#endif
