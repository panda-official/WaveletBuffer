// Copyright 2021-2022 PANDA GmbH

#include <wavelet_buffer/padding.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

using drift::PaddingLocation;
using drift::ZeroDerivativePaddingAlgorithm;
using drift::ZeroPaddingAlgorithm;

TEST_CASE("ZeroPadMatrix::Extend()", "[wavelet]") {
  const blaze::DynamicMatrix<float> kMatrix{{1, 2, 3}, {4, 5, 6}};
  SECTION("location = kRight") {
    ZeroPaddingAlgorithm padding(4, 5, PaddingLocation::kRight);
    const blaze::DynamicMatrix<float> kExpected{
        {1, 2, 3, 0, 0},
        {4, 5, 6, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    };

    const auto result = padding.Extend(kMatrix);
    REQUIRE(result == kExpected);
  }

  SECTION("location = kBoth") {
    ZeroPaddingAlgorithm padding(4, 5, PaddingLocation::kBoth);

    const blaze::DynamicMatrix<float> kExpected{
        {0, 0, 0, 0, 0},
        {0, 1, 2, 3, 0},
        {0, 4, 5, 6, 0},
        {0, 0, 0, 0, 0},
    };

    const auto result = padding.Extend(kMatrix);
    REQUIRE(result == kExpected);
  }
}

TEST_CASE("ZeroPaddingAlgorithm::Extend()_1D", "[wavelet]") {
  const blaze::DynamicMatrix<float> kMatrix{{1}, {2}};
  SECTION("location == kRight") {
    ZeroPaddingAlgorithm padding(4, 1, PaddingLocation::kRight);
    const blaze::DynamicMatrix<float> kExpected{{1}, {2}, {0}, {0}};
    const auto result = padding.Extend(kMatrix);
    REQUIRE(result == kExpected);
  }
  SECTION("location == kBoth") {
    ZeroPaddingAlgorithm padding(4, 1, PaddingLocation::kBoth);
    const blaze::DynamicMatrix<float> kExpected{{0}, {1}, {2}, {0}};
    const auto result = padding.Extend(kMatrix);
    REQUIRE(result == kExpected);
  }
}

TEST_CASE("ZeroPaddingAlgorithm::Crop()", "[wavelet]") {
  const blaze::DynamicMatrix<float> kMatrix{
      {1, 2, 3},
      {4, 5, 6},
      {7, 8, 9},
  };

  SECTION("location = kRight") {
    ZeroPaddingAlgorithm padding(2, 2, PaddingLocation::kRight);

    const blaze::DynamicMatrix<float> kExpected{{1, 2}, {4, 5}};

    const auto kResult = padding.Crop(kMatrix);
    REQUIRE(kResult == kExpected);
  }

  SECTION("location = kBoth") {
    ZeroPaddingAlgorithm padding(1, 2, PaddingLocation::kBoth);

    const blaze::DynamicMatrix<float> kExpected{{4, 5}};

    const auto kResult = padding.Crop(kMatrix);
    REQUIRE(kResult == kExpected);
  }
}

TEST_CASE("ZeroPaddingAlgorithm::Crop()_1D", "[wavelet]") {
  const blaze::DynamicMatrix<float> kMatrix{{1}, {2}, {3}, {4}, {5}};

  SECTION("location = kRight") {
    ZeroPaddingAlgorithm padding(2, 1, PaddingLocation::kRight);

    const blaze::DynamicMatrix<float> kExpected{{1}, {2}};

    const auto kResult = padding.Crop(kMatrix);
    REQUIRE(kResult == kExpected);
  }

  SECTION("location = kBoth") {
    ZeroPaddingAlgorithm padding(2, 1, PaddingLocation::kBoth);

    const blaze::DynamicMatrix<float> kExpected{{2}, {3}};

    const auto kResult = padding.Crop(kMatrix);
    REQUIRE(kResult == kExpected);
  }
}

TEST_CASE("ZeroPadMatrix empty case", "[wavelet]") {
  const blaze::DynamicMatrix<float> kEmptyMatrix{0, 0};

  auto location = GENERATE(PaddingLocation::kRight, PaddingLocation::kBoth);
  CAPTURE(location);

  ZeroPaddingAlgorithm padding(0, 0, location);

  const auto result = padding.Extend(kEmptyMatrix);
  REQUIRE(result == kEmptyMatrix);
  REQUIRE(padding.Crop(result) == kEmptyMatrix);
}

TEST_CASE("ZeroDerivativePaddingAlgorithm::Extend()", "[wavelet]") {
  const blaze::DynamicMatrix<float> kMatrix{
      {1, 2, 3},
      {4, 5, 6},
  };

  SECTION("location = kRight") {
    ZeroDerivativePaddingAlgorithm padding(4, 5, PaddingLocation::kRight);

    const blaze::DynamicMatrix<float> kExpected{
        {1, 2, 3, 3, 3},
        {4, 5, 6, 6, 6},
        {4, 5, 6, 6, 6},
        {4, 5, 6, 6, 6},
    };

    const auto kResult = padding.Extend(kMatrix);
    REQUIRE(kResult == kExpected);
  }

  SECTION("location = kBoth") {
    ZeroDerivativePaddingAlgorithm padding(4, 6, PaddingLocation::kBoth);

    const blaze::DynamicMatrix<float> kExpected{
        {1, 1, 2, 3, 3, 3},
        {1, 1, 2, 3, 3, 3},
        {4, 4, 5, 6, 6, 6},
        {4, 4, 5, 6, 6, 6},
    };

    const auto kResult = padding.Extend(kMatrix);
    REQUIRE(kResult == kExpected);
  }
}

TEST_CASE("ZeroDerivativePaddingAlgorithm::Extend()_1D", "[wavelet]") {
  const blaze::DynamicMatrix<float> kMatrix{{1}, {4}};

  SECTION("location = kRight") {
    ZeroDerivativePaddingAlgorithm padding(5, 1, PaddingLocation::kRight);

    const blaze::DynamicMatrix<float> kExpected{{1}, {4}, {4}, {4}, {4}};

    const auto kResult = padding.Extend(kMatrix);
    REQUIRE(kResult == kExpected);
  }

  SECTION("location = kBoth") {
    ZeroDerivativePaddingAlgorithm padding(5, 1, PaddingLocation::kBoth);

    const blaze::DynamicMatrix<float> kExpected{{1}, {1}, {4}, {4}, {4}};

    const auto kResult = padding.Extend(kMatrix);
    REQUIRE(kResult == kExpected);
  }
}

TEST_CASE("ZeroDerivativePaddingAlgorithm empty case", "[wavelet]") {
  const blaze::DynamicMatrix<float> kEmptyMatrix{0, 0};

  auto location = GENERATE(PaddingLocation::kRight, PaddingLocation::kBoth);
  CAPTURE(location);

  ZeroDerivativePaddingAlgorithm padding(0, 0, location);

  const auto result = padding.Extend(kEmptyMatrix);
  REQUIRE(result == kEmptyMatrix);
  REQUIRE(padding.Crop(result) == kEmptyMatrix);
}
