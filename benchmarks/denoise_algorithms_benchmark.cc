// Copyright 2020-2021 PANDA GmbH

#include <wavelet_buffer/denoise_algorithms.h>

#include <fstream>

#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Denoise algorithms benchmark (pic 500x500)") {
  std::normal_distribution<double> normal_distribution;
  std::mt19937 random_engine;

  auto a = blaze::generate<blaze::rowMajor>(
      500, 500, [&random_engine, &normal_distribution](size_t i, size_t j) {
        return normal_distribution(random_engine);
      });

  SECTION("Simple denoise algorithm") {
    BENCHMARK("Threshold 0.25") {
      drift::SimpleDenoiseAlgorithm<double> da(0.25);
      return da.Denoise(a);
    };

    BENCHMARK("Threshold 0.5") {
      drift::SimpleDenoiseAlgorithm<double> da(0.5);
      return da.Denoise(a);
    };

    BENCHMARK("Threshold 0.75") {
      drift::SimpleDenoiseAlgorithm<double> da(0.75);
      return da.Denoise(a);
    };
  }
}
