// Copyright 2020-2022 PANDA GmbH

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>

#include <fstream>

#include <wavelet_buffer/img/jpeg_codecs.h>

using ::drift::img::HslJpegCodec;
using ::drift::SignalN2D;

const std::string kPic100x100("img/fixtures/pic100x100.jpg");
const std::string kPic200x200("img/fixtures/pic200x200.jpg");
const std::string kPic400x400("img/fixtures/pic400x400.jpg");

TEST_CASE("JPEG codec benchmarks") {
  auto load_picture = [](const std::string& path) {
    std::ifstream data(path);
    assert(data);
    std::stringstream ss;
    ss << data.rdbuf();
    return ss.str();
  };

  auto pic_100_x_100 = load_picture(kPic100x100);
  auto pic_200_x_200 = load_picture(kPic200x200);
  auto pic_400_x_400 = load_picture(kPic400x400);

  HslJpegCodec codec;

  SECTION("Decode") {
    SignalN2D img;
    BENCHMARK("jpeg 100x100") { return codec.Decode(pic_100_x_100, &img); };
    BENCHMARK("jpeg 200x200") { return codec.Decode(pic_200_x_200, &img); };
    BENCHMARK("jpeg 400x400") { return codec.Decode(pic_400_x_400, &img); };
  }

  SignalN2D mat_100_x_100;
  REQUIRE(codec.Decode(pic_100_x_100, &mat_100_x_100));
  SignalN2D mat_200_x_200;
  REQUIRE(codec.Decode(pic_200_x_200, &mat_200_x_200));
  SignalN2D mat_400_x_400;
  REQUIRE(codec.Decode(pic_400_x_400, &mat_400_x_400));

  SECTION("Encode") {
    std::string blob;
    BENCHMARK("blaze 100x100") { return codec.Encode(mat_100_x_100, &blob); };
    BENCHMARK("blaze 200x200") { return codec.Encode(mat_200_x_200, &blob); };
    BENCHMARK("blaze 400x400") { return codec.Encode(mat_400_x_400, &blob); };
  }
}
