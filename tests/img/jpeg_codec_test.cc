// Copyright 2020-2022 PANDA GmbH

#include <fstream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "wavelet_buffer/img/jpeg_codecs.h"

using drift::Signal2D;
using drift::SignalN2D;
using drift::img::GrayJpegCodec;
using drift::img::HslJpegCodec;
using drift::img::IImageCodec;
using drift::img::RgbJpegCodec;

namespace bl = blaze;

#define IMAGE_DELTA(a, b) (bl::max(bl::max(bl::abs((a) - (b)))))
#define ASSERT_IMAGE_CLOSE(a, b, delta) REQUIRE(IMAGE_DELTA((a), (b)) < delta)

const auto kImagePath = "fixtures/pandas.jpg";

const SignalN2D kImage = {
    {{0.1, 0.2}, {0.3, 0.4}, {0.5, 0.6}},
    {{0.6, 0.4}, {0.1, 0.2}, {0.5, 0.3}},
    {{0.6, 0.4}, {0.1, 0.2}, {0.5, 0.3}},
    {{0.5, 0.2}, {0.1, 0.6}, {0.3, 0.4}},
};

std::array<std::unique_ptr<IImageCodec>, 3> codecs{
    std::make_unique<HslJpegCodec>(),
    std::make_unique<RgbJpegCodec>(),
    std::make_unique<GrayJpegCodec>(),
};

std::array<size_t, 3> channel_numbers = {3, 3, 1};

TEST_CASE("JpegCodec", "[generators]") {
  auto codec_num = GENERATE(0, 1, 2);
  CAPTURE(codec_num);

  SignalN2D res_image(3);
  std::string blob;

  SECTION("should encode and decode one-pixel image") {
    const SignalN2D image = {{{0.1}}, {{0.2}}, {{0.3}}};
    res_image = image;  // for Gray codec it works only with 1 channel

    REQUIRE(codecs[codec_num]->Encode(image, &blob));
    REQUIRE(codecs[codec_num]->Decode(blob, &res_image));

    ASSERT_IMAGE_CLOSE(res_image, image, 1.f);
  }

  SECTION("should provide channel number") {
    REQUIRE(channel_numbers[codec_num] == codecs[codec_num]->channel_number());
  }

  SECTION("should encode and decode image") {
    res_image = kImage;  // for Gray codec it works only with 1 channel

    SECTION("starting with 0 channel") {
      REQUIRE(codecs[codec_num]->Encode(kImage, &blob));
      REQUIRE(codecs[codec_num]->Decode(blob, &res_image));

      ASSERT_IMAGE_CLOSE(res_image, kImage, 1.f);
    }

    SECTION("starting with any channel") {
      const SignalN2D ch4_image{
          {{0, 0}, {0, 0}, {0, 0}}, kImage[0], kImage[1], kImage[2],
          {{0, 0}, {0, 0}, {0, 0}},
      };
      res_image = ch4_image;

      REQUIRE(codecs[codec_num]->Encode(ch4_image, &blob, 1));
      REQUIRE(codecs[codec_num]->Decode(blob, &res_image, 1));

      CAPTURE(res_image);
      ASSERT_IMAGE_CLOSE(res_image, ch4_image, 1.f);
    }
  }

  SECTION("fails") {
    SignalN2D empty_image = {};
    std::string empty_string;
    SECTION("to decode an empty string") {
      REQUIRE_FALSE(codecs[codec_num]->Decode(empty_string, &empty_image));
    }

    SECTION("to encode no channel") {
      REQUIRE_FALSE(codecs[codec_num]->Encode(empty_image, &empty_string));
    }

    SECTION("to encode wrong channel count") {
      SignalN2D image = {{}, {}};
      REQUIRE_FALSE(codecs[codec_num]->Encode(image, &empty_string));
    }

    SECTION("to encode empty image") {
      SignalN2D image = {{}, {}, {}};
      REQUIRE_FALSE(codecs[codec_num]->Encode(image, &empty_string));
    }
  }

  SECTION("throw exception if quality is out of rage") {
    REQUIRE_THROWS_AS(HslJpegCodec(-0.1), std::runtime_error);
    REQUIRE_THROWS_AS(HslJpegCodec(1.1), std::runtime_error);
    REQUIRE_THROWS_AS(RgbJpegCodec(-0.1), std::runtime_error);
    REQUIRE_THROWS_AS(RgbJpegCodec(1.1), std::runtime_error);
  }
}

TEST_CASE("3-channeled JPEG codec") {
  auto codec_num = GENERATE(0, 1);
  CAPTURE(codec_num);

  SignalN2D empty_image = {};
  std::string blob;

  SECTION("fails to encode channels of different size") {
    Signal2D ch_11 = {{0.f}};
    Signal2D ch_12 = {{0.f}, {0.f}};
    Signal2D ch_21 = {{0.f, 0.f}};

    REQUIRE_FALSE(codecs[codec_num]->Encode({ch_12, ch_11, ch_11}, &blob));
    REQUIRE_FALSE(codecs[codec_num]->Encode({ch_11, ch_12, ch_11}, &blob));
    REQUIRE_FALSE(codecs[codec_num]->Encode({ch_11, ch_11, ch_12}, &blob));

    REQUIRE_FALSE(codecs[codec_num]->Encode({ch_21, ch_11, ch_11}, &blob));
    REQUIRE_FALSE(codecs[codec_num]->Encode({ch_11, ch_21, ch_11}, &blob));
    REQUIRE_FALSE(codecs[codec_num]->Encode({ch_11, ch_11, ch_21}, &blob));
  }

  SECTION("should encode to empty blaze image") {
    std::ifstream img_file(kImagePath);

    REQUIRE(img_file);

    std::stringstream ss;
    ss << img_file.rdbuf();

    SECTION("start_channel = 0") {
      SignalN2D img;
      REQUIRE(codecs[codec_num]->Decode(ss.str(), &img, 0));
      REQUIRE(img.size() == 3);
    }

    SECTION("start_channel = 1") {
      SignalN2D img;
      REQUIRE(codecs[codec_num]->Decode(ss.str(), &img, 1));
      REQUIRE(img.size() == 4);
    }
  }
}

TEST_CASE("Gray JPEG codec") {
  GrayJpegCodec codec;
  SECTION("should encode/decode start_channel=1") {
    std::string data;
    REQUIRE(codec.Encode(kImage, &data, 1));

    SignalN2D new_img;
    REQUIRE(codec.Decode(data, &new_img, 1));

    REQUIRE(new_img.size() == 2);
    REQUIRE_THAT(blaze::norm(kImage[1]),
                 Catch::Matchers::WithinAbs(0.01, blaze::norm(new_img[1])));
  }
}
