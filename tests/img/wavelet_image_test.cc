// Copyright 2020-2022 PANDA GmbH

#include "wavelet_buffer/wavelet_image.h"

#include <fstream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "wavelet_buffer/img/jpeg_codecs.h"
#include "wavelet_buffer/wavelet_buffer_view.h"

using drift::Distance;
using drift::NullDenoiseAlgorithm;
using drift::SimpleDenoiseAlgorithm;
using drift::img::WaveletImage;
using drift::WaveletParameters;
using drift::WaveletTypes;
using drift::img::HslJpegCodec;

const WaveletParameters kWaveletParam{
    .signal_shape = {800, 500},
    .signal_number = 3,
    .decomposition_steps = 4,
    .wavelet_type = WaveletTypes::kDB2,
};

const auto kImagePath = "fixtures/pandas.jpg";
const auto kDumpPath = "dump.bin";

TEST_CASE("Wavelet Image") {
  HslJpegCodec codec;
  NullDenoiseAlgorithm<float> denoiser;

  SECTION("should import and export JPEG file") {
    WaveletImage image(kWaveletParam);
    auto status = image.ImportFromFile(kImagePath, denoiser, codec);

    REQUIRE(WaveletImage::Status::kOk == status);

    status = image.ExportToFile("pandas_out.jpg", codec);
    REQUIRE(WaveletImage::Status::kOk == status);

    WaveletImage new_images(kWaveletParam);

    status = new_images.ImportFromFile("pandas_out.jpg", denoiser, codec);
    REQUIRE(WaveletImage::Status::kOk == status);
    REQUIRE_THAT(new_images.CompareTo(image),
                 Catch::Matchers::WithinAbs(0, 0.006));
  }

  SECTION("fails") {
    WaveletImage image(kWaveletParam);

    SECTION("wrong_channel") {
      auto status = image.ImportFromFile("pandas_out.jpg", denoiser, codec, 1);
      REQUIRE(WaveletImage::Status::kDecompositionError == status);
      status = image.ExportToFile("file.jpeg", codec, 3);
      REQUIRE(WaveletImage::Status::kCompositionError == status);
    }

    SECTION("to read file") {
      auto status = image.ImportFromFile("NO_EXISTING_FILE", denoiser, codec);
      REQUIRE(WaveletImage::Status::kIOError == status);
    }

    SECTION("to write file") {
      auto status = image.ExportToFile("/NO_EXISTING_PATH/file.jpeg", codec);
      REQUIRE(WaveletImage::Status::kIOError == status);
    }
  }

  SECTION("should save and load buffer") {
    WaveletImage image(kWaveletParam);

    auto status = image.ImportFromFile(kImagePath, denoiser, codec);

    REQUIRE(WaveletImage::Status::kOk == status);

    status = image.Save(kDumpPath);
    REQUIRE(WaveletImage::Status::kOk == status);

    auto new_image = WaveletImage::Load(kDumpPath);
    REQUIRE(image.const_buffer() == new_image->const_buffer());
  }

  SECTION("should store few images as channels in WaveletBuffer") {
    auto two_img_param = kWaveletParam;
    two_img_param.signal_number = 6;

    WaveletImage image2(two_img_param);

    REQUIRE(image2.ImportFromFile(kImagePath, denoiser, codec, 3) ==
            WaveletImage::Status::kOk);
    REQUIRE(image2.ExportToFile("pandas_out.jpg", codec, 3) ==
            WaveletImage::Status::kOk);

    WaveletImage image(kWaveletParam);
    REQUIRE(image.ImportFromFile("pandas_out.jpg", denoiser, codec) ==
            WaveletImage::Status::kOk);

    REQUIRE_THAT(Distance(image2.const_buffer()(3, 3), image.const_buffer()),
                 Catch::Matchers::WithinAbs(0, 0.006));
  }
}
