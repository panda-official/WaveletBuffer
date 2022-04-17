// Copyright 2021 PANDA GmbH
#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>

#include <wavelet_buffer/wavelet_buffer.h>

using drift::dsp::EnergyDistribution;
using drift::dsp::SignalN2D;
using drift::dsp::SimpleDenoiseAlgorithm;
using drift::dsp::WaveletBuffer;
using drift::dsp::WaveletParameters;
using drift::dsp::WaveletTypes;

TEST_CASE("Wavelet Util Functions") {
  std::normal_distribution<double> normal_distribution;
  std::mt19937 random_engine;

  WaveletParameters params = {
      .signal_shape = {500, 500},
      .signal_number = 1,
      .decomposition_steps = 3,
      .wavelet_type = WaveletTypes::kDB3,
  };

  auto a = blaze::generate<blaze::rowMajor>(
      params.signal_shape[0], params.signal_shape[1],
      [&random_engine, &normal_distribution](size_t i, size_t j) {
        return normal_distribution(random_engine);
      });

  SignalN2D data = {a};
  WaveletBuffer buffer(params);
  REQUIRE(buffer.Decompose(data, SimpleDenoiseAlgorithm<float>(0.8)));

  BENCHMARK("EnergyDistribution") {
    auto ret = EnergyDistribution(buffer);
    REQUIRE(ret.size() > 0);
    return ret;
  };
}
