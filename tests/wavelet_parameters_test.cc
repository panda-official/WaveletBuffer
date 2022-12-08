// Copyright 2020-2022 PANDA GmbH

#include <wavelet_buffer/wavelet_parameters.h>

#include <catch2/catch_test_macros.hpp>
#include <cereal/archives/portable_binary.hpp>

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

    WaveletParameters params_n2d = params_2d;
    params_n2d.signal_number = 3;
    REQUIRE(params_n2d.dimension() == 2);
  }

  SECTION("Should return shortest side") {
    REQUIRE(params_3d.shortest_dimension() == 100);
  }

  SECTION("Should compare with respect to shape") {
    WaveletParameters big = params_3d;
    big.signal_shape[1] += 1;

    REQUIRE(params_3d < big);
    REQUIRE_FALSE(params_3d > big);
    REQUIRE_FALSE(params_3d == big);
  }

  SECTION("Should compare with respect to signal number") {
    WaveletParameters big = params_3d;
    big.signal_number += 1;

    REQUIRE(params_3d < big);
    REQUIRE_FALSE(params_3d > big);
    REQUIRE_FALSE(params_3d == big);
  }

  SECTION("Should compare with respect to decomposition steps") {
    WaveletParameters big = params_3d;
    big.decomposition_steps += 1;

    REQUIRE(params_3d < big);
    REQUIRE_FALSE(params_3d > big);
    REQUIRE_FALSE(params_3d == big);
  }

  SECTION("Should compare with respect to wavelet type") {
    WaveletParameters big = params_3d;
    big.wavelet_type = drift::kDB4;

    REQUIRE(params_3d < big);
    REQUIRE_FALSE(params_3d > big);
    REQUIRE_FALSE(params_3d == big);
  }
}

TEST_CASE("WaveletParameters::serialization()") {
  WaveletParameters p;

  std::stringstream ss;

  {
    cereal::PortableBinaryOutputArchive archive(ss);
    archive(p);
  }

  WaveletParameters p2;
  {
    cereal::PortableBinaryInputArchive archive(ss);
    archive(p2);
  }

  REQUIRE(p == p2);
}
