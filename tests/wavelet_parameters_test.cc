// Copyright 2020-2022 PANDA GmbH

#include <catch2/catch_test_macros.hpp>
#include <wavelet_buffer/wavelet_parameters.h>

using drift::WaveletParameters;
using drift::WaveletTypes;

TEST_CASE("WaveletParameters") {
  const WaveletParameters params_1d{
      .signal_shape = {100},
      .signal_number = 1,
      .decomposition_steps = 10,
      .wavelet_type = WaveletTypes::kDB3,
  };

  const WaveletParameters params_2d{
      .signal_shape = {100, 200},
      .signal_number = 1,
      .decomposition_steps = 10,
      .wavelet_type = WaveletTypes::kDB3,
  };

  const WaveletParameters params_3d{
      .signal_shape = {200, 100, 300},
      .signal_number = 1,
      .decomposition_steps = 10,
      .wavelet_type = WaveletTypes::kDB3,
  };

  SECTION("Should return number of dimensions") {
    REQUIRE(params_1d.dimension() == 1);
    REQUIRE(params_2d.dimension() == 2);
    REQUIRE(params_3d.dimension() == 3);

    auto params_n2d = params_2d;
    params_n2d.signal_number = 3;
    REQUIRE(params_n2d.dimension() == 2);
  }

  SECTION("Should return shortest side") {
    REQUIRE(params_3d.shortest_dimension() == 100);
  }

  SECTION("Should compare with respect to shape") {
    auto small = params_3d;
    auto big = params_3d;
    big.signal_shape[1] += 1;

    REQUIRE(small < big);
    REQUIRE_FALSE(small > big);
    REQUIRE_FALSE(small == big);
  }

  SECTION("Should compare with respect to signal number") {
    auto small = params_3d;
    auto big = params_3d;
    big.signal_number += 1;

    REQUIRE(small < big);
    REQUIRE_FALSE(small > big);
    REQUIRE_FALSE(small == big);
  }

  SECTION("Should compare with respect to decomposition steps") {
    auto small = params_3d;
    auto big = params_3d;
    big.decomposition_steps += 1;

    REQUIRE(small < big);
    REQUIRE_FALSE(small > big);
    REQUIRE_FALSE(small == big);
  }

  SECTION("Should compare with respect to wavelet type") {
    auto small = params_3d;
    auto big = params_3d;
    big.wavelet_type = drift::kDB4;

    REQUIRE(small < big);
    REQUIRE_FALSE(small > big);
    REQUIRE_FALSE(small == big);
  }
}
