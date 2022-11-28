// Copyright 2020-2022 PANDA GmbH

#include "wavelet_buffer/img/color_space.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using drift::img::ConvertRgbToHsl;
using drift::img::HSLToRGB;
using drift::img::RGBToHSL;

struct RgbHslPixel {
  float r;
  float g;
  float b;
  float h;
  float s;
  float l;
};

const RgbHslPixel kBlack = {.r = 0, .g = 0, .b = 0, .h = 0, .s = 0, .l = 0};
const RgbHslPixel kWhite = {.r = 1, .g = 1, .b = 1, .h = 0, .s = 0, .l = 1};
const RgbHslPixel kRed = {.r = 1, .g = 0, .b = 0, .h = 0, .s = 1, .l = 0.5};
const RgbHslPixel kLime = {
    .r = 0, .g = 1, .b = 0, .h = 1 / 3.f, .s = 1, .l = 0.5};
const RgbHslPixel kBlue = {
    .r = 0, .g = 0, .b = 1, .h = 2 / 3.f, .s = 1, .l = 0.5};
const RgbHslPixel kYellow = {
    .r = 1, .g = 1, .b = 0, .h = 1 / 6.f, .s = 1, .l = 0.5};
const RgbHslPixel kCyan = {.r = 0, .g = 1, .b = 1, .h = 0.5f, .s = 1, .l = 0.5};
const RgbHslPixel kMagenta = {
    .r = 1, .g = 0, .b = 1, .h = 5 / 6.f, .s = 1, .l = 0.5};
const RgbHslPixel kGray = {.r = 128 / 255.f,
                           .g = 128 / 255.f,
                           .b = 128 / 255.f,
                           .h = 0,
                           .s = 0,
                           .l = 0.5};

TEST_CASE("RGB <-> HSL color space conversion") {
  const float abs_error = 0.002;
  auto pixel = GENERATE(kWhite, kBlack, kRed, kLime, kBlue, kYellow, kCyan,
                        kMagenta, kGray);

  SECTION("RGB -> HSL") {
    float h, s, l;
    RGBToHSL(pixel.r, pixel.g, pixel.b, &h, &s, &l);

    REQUIRE_THAT(h, Catch::Matchers::WithinAbs(pixel.h, abs_error));
    REQUIRE_THAT(s, Catch::Matchers::WithinAbs(pixel.s, abs_error));
    REQUIRE_THAT(l, Catch::Matchers::WithinAbs(pixel.l, abs_error));
  }

  SECTION("HSL -> RGB") {
    float r, g, b;
    HSLToRGB(pixel.h, pixel.s, pixel.l, &r, &g, &b);

    REQUIRE_THAT(r, Catch::Matchers::WithinAbs(pixel.r, abs_error));
    REQUIRE_THAT(g, Catch::Matchers::WithinAbs(pixel.g, abs_error));
    REQUIRE_THAT(b, Catch::Matchers::WithinAbs(pixel.b, abs_error));
  }

  SECTION("RGB image -> HSL image") {
    using Image = blaze::DynamicVector<blaze::DynamicMatrix<float>>;

    std::normal_distribution<double> normal_distribution;
    std::mt19937 random_engine;
    auto random_generator = [&random_engine, &normal_distribution](size_t i,
                                                                   size_t j) {
      return normal_distribution(random_engine);
    };

    Image image{
        blaze::generate<blaze::rowMajor>(10, 10, random_generator),
        blaze::generate<blaze::rowMajor>(10, 10, random_generator),
        blaze::generate<blaze::rowMajor>(10, 10, random_generator),
    };

    auto copy = image;

    ConvertRgbToHsl(&image);

    for (int y = 0; y < image[0].rows(); ++y) {
      for (int x = 0; x < image[0].columns(); ++x) {
        float h, s, l;
        RGBToHSL(copy[0](y, x), copy[1](y, x), copy[2](y, x), &h, &s, &l);
        REQUIRE(h == image[0](y, x));
        REQUIRE(s == image[1](y, x));
        REQUIRE(l == image[2](y, x));
      }
    }
  }
}
