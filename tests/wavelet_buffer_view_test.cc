// Copyright 2021 PANDA GmbH

#include <wavelet_buffer/wavelet_buffer_view.h>

#include <catch2/catch_test_macros.hpp>

using ::drift::dsp::Signal2D;
using ::drift::dsp::SignalN2D;
using ::drift::dsp::WaveletBuffer;
using ::drift::dsp::WaveletBufferView;
using ::drift::dsp::WaveletDecomposition;
using ::drift::dsp::WaveletParameters;
using ::drift::dsp::WaveletTypes;
using Denoiser = ::drift::dsp::NullDenoiseAlgorithm<float>;

TEST_CASE("Wavelet Buffer View 2D", "[generators]") {
  WaveletBuffer buffer(WaveletParameters{
      .signal_shape = {5, 5},
      .signal_number = 4,
      .decomposition_steps = 2,
      .wavelet_type = WaveletTypes::kDB1,
  });

  const Signal2D kChannel = {
      {0, 1, 2, 3, 4},      {5, 6, 7, 8, 9},      {10, 11, 12, 13, 14},
      {15, 16, 17, 18, 19}, {20, 21, 22, 23, 24},
  };

  const SignalN2D kNImage = {
      kChannel,
      kChannel + 1,
      kChannel + 2,
      kChannel + 3,
  };

  const WaveletDecomposition kEmptyDecompos(7);
  Denoiser denoiser;
  SignalN2D data;

  auto view = buffer(1, 1);

  SECTION("should decompose signal") {
    REQUIRE(view.Decompose({kChannel + 1}, denoiser));
    REQUIRE(view.decompositions().size() == 1);

    SECTION("correctly") {
      WaveletBuffer test_buf(buffer.parameters());
      test_buf.Decompose(kNImage, denoiser);

      REQUIRE(test_buf.decompositions()[1] == view.decompositions()[0]);
    }

    SECTION("change part of buffer") {
      REQUIRE(buffer.decompositions()[0] == kEmptyDecompos);
      REQUIRE(buffer.decompositions()[1] != kEmptyDecompos);
      REQUIRE(buffer.decompositions()[2] == kEmptyDecompos);

      REQUIRE(buffer.decompositions()[1] == view.decompositions()[0]);
    }
  }

  SECTION("fails") {
    SECTION("if the view has wrong size") {
      REQUIRE_FALSE(buffer(1, 5).Decompose(kNImage, denoiser));
      REQUIRE_FALSE(buffer(1, 5).Compose(&data));
    }

    SECTION("if signal has a bad shape") {
      auto wrong_size = kChannel;
      blaze::resize(wrong_size, 2, 2);
      REQUIRE_FALSE(view.Decompose({wrong_size}, denoiser));

      REQUIRE(buffer.decompositions()[1] == kEmptyDecompos);
    }
  }

  SECTION("should compose signal") {
    buffer.Decompose(kNImage, denoiser);

    SECTION("correctly") {
      REQUIRE(view.Compose(&data));
      REQUIRE(data.size() == 1);
      REQUIRE(data[0] == (kChannel + 1));
    }
  }

  SECTION("should cast to buffer with copping") {
    buffer.Decompose(kNImage, denoiser);
    WaveletBuffer new_buffer = view;
    REQUIRE(new_buffer.decompositions()[0] == buffer.decompositions()[1]);
  }
}
