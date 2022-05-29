// Copyright 2020 PANDA GmbH

#include <wavelet_buffer/denoise_algorithms.h>

#include <catch2/catch_test_macros.hpp>

//#include "drift/utils/utils.h"

const blaze::DynamicMatrix<float> kSource2D = {
    {0, 255, 10, 0, 0},   {0, 255, 9, 127, 0}, {0, 255, 2, 127, 0},
    {0, 255, 10, 127, 0}, {0, 255, 0, 127, 0},
};

const blaze::DynamicVector<float> kSource1D = {0,   255, 10, 20,  30, 127, 60,
                                               240, 80,  10, 0,   3,  8,   250,
                                               25,  187, 90, 252, 2,  1};

TEST_CASE("Denoise Algorithms") {
  SECTION("Null Algorithm should do nothing") {
    drift::dsp::NullDenoiseAlgorithm<float> denoiser;
    REQUIRE(kSource2D == denoiser.Denoise(kSource2D));
  }
  SECTION("Algorithms for 2D signal") {
    SECTION("Simple Algorithm should remove 80% of the smallest values") {
      drift::dsp::SimpleDenoiseAlgorithm<float> denoiser(0.8);

      const blaze::DynamicMatrix<float> kResult2D = {
          {0, 255, 0, 0, 0}, {0, 255, 0, 0, 0}, {0, 255, 0, 0, 0},
          {0, 255, 0, 0, 0}, {0, 255, 0, 0, 0},
      };

      REQUIRE(kResult2D == denoiser.Denoise(kSource2D));
    }

    SECTION(
        "ThresholdAbs algorithm must remove all values lower than threshold by "
        "abs") {
      const blaze::DynamicMatrix<float> kSample = {{-1, -4.5, 0}, {0, 10, 1.5}};

      drift::dsp::ThresholdAbsDenoiseAlgorithm<float> denoiser(0.5, -3);

      SECTION("Step = 0") {
        const blaze::DynamicMatrix<float> kExpect = {{-1, -4.5, 0},
                                                     {0, 10, 1.5}};

        REQUIRE(kExpect == denoiser.Denoise(kSample));
      }
      SECTION("Step = 2") {
        const blaze::DynamicMatrix<float> kExpect = {{-1, -4.5, 0},
                                                     {0, 10, 1.5}};

        REQUIRE(kExpect == denoiser.Denoise(kSample, 2));
      }
      SECTION("Step = 10") {
        const blaze::DynamicMatrix<float> kExpect = {{0, -4.5, 0}, {0, 10, 0}};

        REQUIRE(kExpect == denoiser.Denoise(kSample, 10));
      }
    }
  }

  SECTION("Algorithms for 1D signal") {
    SECTION("Simple Algorithm should remove 80% of the smallest values") {
      drift::dsp::SimpleDenoiseAlgorithm<float> denoiser(0.8);

      const blaze::DynamicVector<float> kResult1D = {
          0, 255, 0, 0, 0, 0, 0, 240, 0, 0, 0, 0, 0, 250, 0, 0, 0, 252, 0, 0};

      REQUIRE(kResult1D == denoiser.Denoise(kSource1D));
    }

    SECTION(
        "ThresholdAbs algorithm must remove all values lower than threshold") {
      const blaze::DynamicVector<float> kSample = {-1, -4.5, 0, 0, 10, 1.5};

      drift::dsp::ThresholdAbsDenoiseAlgorithm<float> denoiser(-1.2, 4);

      SECTION("Step = 0") {
        const blaze::DynamicVector<float> kExpect = {0, -4.5, 0, 0, 10, 0};

        REQUIRE(kExpect == denoiser.Denoise(kSample));
      }
      SECTION("Step = 5") {
        const blaze::DynamicVector<float> kExpect = {-1, -4.5, 0, 0, 10, 1.5};

        REQUIRE(kExpect == denoiser.Denoise(kSample, 5));
      }
      SECTION("Step = 10") {
        const blaze::DynamicVector<float> kExpect = {-1, -4.5, 0, 0, 10, 1.5};

        REQUIRE(kExpect == denoiser.Denoise(kSample, 10));
      }
    }
  }
}
