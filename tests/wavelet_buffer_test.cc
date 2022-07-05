// Copyright 2020-2021 PANDA GmbH

#include "wavelet_buffer/wavelet_buffer.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using drift::dsp::DenoiseAlgorithm;
using drift::dsp::Distance;
using drift::dsp::NullDenoiseAlgorithm;
using drift::dsp::Signal1D;
using drift::dsp::SignalN2D;
using drift::dsp::SimpleDenoiseAlgorithm;
using drift::dsp::Subband;
using drift::dsp::WaveletBuffer;
using drift::dsp::WaveletParameters;
using drift::dsp::WaveletTypes;

static bool Compose(const WaveletBuffer &buffer, SignalN2D *signal,
                    int scale = 0) {
  if (buffer.parameters().signal_shape.size() == 1) {
    return buffer.Compose(&signal[0], scale);
  }
  if ((buffer.parameters().signal_shape.size() == 2)) {
    return buffer.Compose(signal, scale);
  }
  throw std::runtime_error("Failed on Compose: bad test initialization");
}

static bool Decompose(
    WaveletBuffer *buffer, const SignalN2D &signal,
    const DenoiseAlgorithm<float> &denoiser = NullDenoiseAlgorithm<float>()) {
  if (buffer->parameters().signal_shape.size() == 1) {
    Signal1D signal_1d{blaze::column(signal[0], 0)};
    return buffer->Decompose(signal_1d, denoiser);
  }
  if (buffer->parameters().signal_shape.size() == 2) {
    return buffer->Decompose(signal, denoiser);
  }
  throw std::runtime_error("Failed on Decompose: bad test initialization");
}

/**
 * Construct WaveletParameters
 */
WaveletParameters MakeParams(const std::vector<size_t> &shape,
                             const size_t decomposition_steps,
                             const WaveletTypes type = WaveletTypes::kDB2) {
  return WaveletParameters{
      .signal_shape = shape,
      .signal_number = 1,
      .decomposition_steps = decomposition_steps,
      .wavelet_type = type,
  };
}

class DataGenerator {
  std::default_random_engine random_engine_;
  std::normal_distribution<float> normal_distribution_;

 public:
  blaze::DynamicVector<float> GenerateMatrix1d(size_t i);
  blaze::DynamicMatrix<float> GenerateMatrix2d(size_t rows, size_t cols);
};

blaze::DynamicVector<float> DataGenerator::GenerateMatrix1d(size_t i) {
  return blaze::generate<blaze::columnVector>(
      100, [this](size_t i) { return normal_distribution_(random_engine_); });
}

blaze::DynamicMatrix<float> DataGenerator::GenerateMatrix2d(size_t rows,
                                                            size_t cols) {
  return blaze::generate<blaze::rowMajor>(
      rows, cols, [this](size_t i, size_t j) {
        return normal_distribution_(random_engine_);
      });
}

TEST_CASE("WaveletBuffer::WaveletBuffer()", "[generators]") {
  NullDenoiseAlgorithm<float> denoiser;
  DataGenerator dg;

  SECTION("should validate parameters") {
    SECTION("wrong number of dimensions") {
      auto params = MakeParams({100, 100, 1}, 1);
      REQUIRE_THROWS_WITH(WaveletBuffer(params),
                          "Only 1D & 2D decomposition is supported");
    }

    SECTION("input size is too small") {
      auto params = MakeParams({1}, 1);
      REQUIRE_THROWS_WITH(WaveletBuffer(params),
                          "Input signal shape is too small");
    }

    SECTION("decomposition steps is too big") {
      auto params = MakeParams({100, 100}, 10);
      REQUIRE_THROWS_WITH(WaveletBuffer(params),
                          "Too many decomposition steps for this signal size "
                          "with that wavelet type (must be max 5).");
    }
  }

  // TODO(victor1234): Fix test

  SECTION("should validate decomposition") {
    const auto shape =
        GENERATE(std::vector<size_t>{100}, std::vector<size_t>{200, 300});
    const auto decomposition_steps = GENERATE(0, 1, 2);

    auto params = MakeParams(shape, decomposition_steps);
    params.signal_number = 3;

    auto buffer = WaveletBuffer(params);

    Decompose(&buffer,
              SignalN2D{params.signal_number,
                        dg.GenerateMatrix2d(shape[0],
                                            shape.size() > 1 ? shape[1] : 1)});

    auto decompositions = buffer.decompositions();

    SECTION("create new buffer correctly") {
      WaveletBuffer new_buffer(params, decompositions);
      REQUIRE(Distance(buffer, new_buffer) == 0);
    }

    SECTION("wrong decomposition size") {
      decompositions.resize(params.signal_number - 1);

      REQUIRE_THROWS_WITH(
          WaveletBuffer(params, decompositions),
          "Wrong signal number in decomposition. Expected 3 but got 2");
    }

    SECTION("wrong subband size") {
      decompositions.at(1).resize(1000);

      REQUIRE_THROWS_WITH(WaveletBuffer(params, decompositions),
                          "Wrong number of subbands in signal 1. Expected " +
                              std::to_string(decompositions.at(0).size()) +
                              " but got 1000");
    }
  }
}

TEST_CASE("WaveletBuffer::IsEmpty()", "[generators]") {
  NullDenoiseAlgorithm<float> denoiser;
  DataGenerator dg;

  SignalN2D signal{dg.GenerateMatrix2d(100, 100)};
  auto params = MakeParams({100, 100}, 2);
  WaveletBuffer buffer(params);

  REQUIRE(buffer.IsEmpty());

  REQUIRE(buffer.Decompose(signal, denoiser));
  REQUIRE_FALSE(buffer.IsEmpty());

  WaveletBuffer buffer2(params, buffer.decompositions());
  REQUIRE_FALSE(buffer2.IsEmpty());
}

TEST_CASE("WaveletBuffer::Parse(), WaveletBuffer::Serialize()",
          "[generators]") {
  DataGenerator dg;

  /* Fails to parse garbage */
  REQUIRE_FALSE(WaveletBuffer::Parse("GARBAGE"));

  // TODO(victor1234): Fix test

  SECTION("should serialize and deserialize") {
    const auto buffer_num = GENERATE(0, 1);
    std::vector<WaveletBuffer> buffers = {
        WaveletBuffer(MakeParams({10000}, 2)),
        WaveletBuffer(MakeParams({100, 100}, 2))};

    auto buffer = buffers[buffer_num];
    std::vector<SignalN2D> signals{SignalN2D{dg.GenerateMatrix2d(10000, 1)},
                                   SignalN2D{dg.GenerateMatrix2d(100, 100)}};

    REQUIRE(Decompose(&buffer, signals[buffer_num],
                      SimpleDenoiseAlgorithm<float>(0.7)));

    SECTION("the same data") {
      auto compression_level =
          GENERATE(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

      std::string blob;
      REQUIRE(buffer.Serialize(&blob, compression_level));

      auto buffer_dst = WaveletBuffer::Parse(blob);
      REQUIRE(buffer_dst);
      REQUIRE(std::fabs(Distance(buffer, *buffer_dst)) < 0.0001);
    }

    SECTION("check version") {
      std::string blob;
      REQUIRE(buffer.Serialize(&blob));
      blob[0] = -2;  // brake first byte

      REQUIRE_FALSE(WaveletBuffer::Parse(blob));
    }

    SECTION("zeros") {
      auto empty_buffer = buffers[buffer_num];
      std::string blob;
      REQUIRE(empty_buffer.Serialize(&blob, 1));

      auto restored_buffer = WaveletBuffer::Parse(blob);
      REQUIRE(restored_buffer);
      REQUIRE(*restored_buffer == empty_buffer);
    }
  }
}
TEST_CASE("Wavelet Buffer") {
  NullDenoiseAlgorithm<float> denoiser;
  DataGenerator dg;

  SECTION("should compose and decompose signal") {
    SECTION("square 2D signal") {
      WaveletBuffer buffer(MakeParams({100, 100}, 2));
      SignalN2D data_dst, signal{dg.GenerateMatrix2d(100, 100)};

      REQUIRE(buffer.Decompose(signal, denoiser));
      REQUIRE(buffer.Compose(&data_dst));
      REQUIRE(signal == data_dst);
    }

    SECTION("rect 2D signal") {
      WaveletBuffer buffer(MakeParams({150, 100}, 2));
      SignalN2D data_dst, signal{dg.GenerateMatrix2d(100, 150)};

      REQUIRE(buffer.Decompose(signal, denoiser));

      SECTION("Scale = 0") {
        REQUIRE(buffer.Compose(&data_dst));
        REQUIRE(signal[0].rows() == data_dst[0].rows());
        REQUIRE(signal[0].columns() == data_dst[0].columns());
        REQUIRE(signal == data_dst);
      }

      SECTION("Scale = 1") {
        REQUIRE(buffer.Compose(&data_dst, 1));
        REQUIRE(signal[0].rows() / 2 == data_dst[0].rows());
        REQUIRE(signal[0].columns() / 2 == data_dst[0].columns());
      }

      SECTION("Scale = 2") {
        REQUIRE(buffer.Compose(&data_dst, 2));
        REQUIRE(signal[0].rows() / 4 == data_dst[0].rows());
        REQUIRE(signal[0].columns() / 4 == data_dst[0].columns());
      }
    }

    SECTION("linear signal") {
      WaveletBuffer buffer(MakeParams({100}, 2));
      Signal1D data_dst, linear_signal{dg.GenerateMatrix1d(100)};

      REQUIRE(buffer.Decompose(linear_signal, denoiser));

      SECTION("Scale = 0") {
        REQUIRE(buffer.Compose(&data_dst));
        REQUIRE(linear_signal == data_dst);
      }

      SECTION("Scale = 1") {
        REQUIRE(buffer.Compose(&data_dst, 1));
        REQUIRE(linear_signal.size() / 2 == data_dst.size());
      }

      SECTION("Scale = 2") {
        REQUIRE(buffer.Compose(&data_dst, 2));
        REQUIRE(linear_signal.size() / 4 == data_dst.size());
      }
    }
  }

  SECTION("should do nothing for 0 steps") {
    SECTION("small signal doesn't work") {
      REQUIRE_THROWS(WaveletBuffer(MakeParams({2, 2}, 0)),
                     std::runtime_error(""));

      REQUIRE_THROWS(WaveletBuffer(MakeParams({2}, 0)), std::runtime_error(""));
    }

    SECTION("1D signal") {
      WaveletBuffer buffer(MakeParams({100}, 0));
      Signal1D signal{dg.GenerateMatrix1d(100)};

      REQUIRE(buffer.Decompose(signal, denoiser));
      REQUIRE(buffer.decompositions()[0].size() == 1);

      Subband expected{100, 1};
      blaze::column(expected, 0) = signal;
      REQUIRE(buffer[0][0] == expected);
    }

    SECTION("2D signal") {
      WaveletBuffer buffer(MakeParams({100, 200}, 0));
      SignalN2D signal{dg.GenerateMatrix2d(200, 100)};

      REQUIRE(buffer.Decompose(signal, denoiser));
      REQUIRE(buffer.decompositions()[0].size() == 1);
      REQUIRE(buffer[0][0] == static_cast<Subband>(signal[0]));
    }
  }

  // TODO(victor1234): Fix test with kNone
  SECTION("should do nothing for type kNone") {
    WaveletParameters param =
        GENERATE(MakeParams({2}, 100, WaveletTypes::kNone),
                 MakeParams({2, 2}, 100, WaveletTypes::kNone),
                 MakeParams({100}, 1000, WaveletTypes::kNone),
                 MakeParams({100, 100}, 2, WaveletTypes::kNone),
                 MakeParams({0}, 1000, WaveletTypes::kNone),
                 MakeParams({0, 0}, 2, WaveletTypes::kNone));

    WaveletBuffer buffer(param);
    SignalN2D signal = SignalN2D{
        dg.GenerateMatrix2d(param.signal_shape[0],
                            param.dimension() == 2 ? param.signal_shape[1] : 1),
    };
    SignalN2D restored_signal;

    REQUIRE(Decompose(&buffer, signal));
    REQUIRE(Compose(buffer, &restored_signal));
    REQUIRE(signal == restored_signal);
  }

  SECTION("should provide max min for specific subband") {
    SECTION("1D signal") {
      WaveletBuffer buf(MakeParams({100}, 2));
      REQUIRE(std::make_pair(-1.0f, 1.0f) == buf.GetValueRange(0));
      REQUIRE(std::make_pair(-2.0f, 2.0f) == buf.GetValueRange(1));
      REQUIRE(std::make_pair(0.0f, 4.0f) == buf.GetValueRange(2));
    }

    SECTION("2D signal") {
      WaveletBuffer buf(MakeParams({100, 100}, 2));
      REQUIRE(std::make_pair(-1.0f, 1.0f) == buf.GetValueRange(0));
      REQUIRE(std::make_pair(-2.0f, 2.0f) == buf.GetValueRange(4));
      REQUIRE(std::make_pair(0.0f, 4.0f) == buf.GetValueRange(6));
    }
  }

  // TODO(victor1234): Fix test

  SECTION("should partial compose buffer") {
    auto buffer_num = GENERATE(0, 1);
    CAPTURE(buffer_num);
    std::vector<WaveletBuffer> buffers1 = {
        WaveletBuffer(MakeParams({100}, 1)),
        WaveletBuffer(MakeParams({100, 100}, 1))};
    std::vector<WaveletBuffer> buffers2 = {
        WaveletBuffer(MakeParams({100}, 2)),
        WaveletBuffer(MakeParams({100, 100}, 2))};
    std::vector<SignalN2D> signals = {SignalN2D{dg.GenerateMatrix2d(100, 1)},
                                      SignalN2D{dg.GenerateMatrix2d(100, 100)}};
    REQUIRE(Decompose(&buffers1[buffer_num], signals[buffer_num]));
    REQUIRE(Decompose(&buffers2[buffer_num], signals[buffer_num]));
  }

  SECTION("should parse hardly compressed blob") {
    using Denoiser = drift::dsp::ThresholdAbsDenoiseAlgorithm<float>;
    auto buffer = WaveletBuffer(MakeParams({100}, 3));
    REQUIRE(buffer.Decompose(Signal1D(100), Denoiser(1000, 1000)));

    std::string blob;
    REQUIRE(buffer.Serialize(&blob));
    REQUIRE(WaveletBuffer::Parse(blob));
  }
}

TEST_CASE("dwt()", "[wavelet]") {
  NullDenoiseAlgorithm<float> denoiser;

  /* Use sizes without padding */
  SECTION("Signal size = 8, db2") {
    Signal1D data = {1, 2, 3, 4, 5, 6, 7, 8};

    WaveletParameters wp = MakeParams({data.size()}, 1);

    WaveletBuffer wb(wp);

    wb.Decompose(data, denoiser);

    const auto a = wb.decompositions()[0][1];
    const auto d = wb.decompositions()[0][0];

    REQUIRE(a == blaze::DynamicMatrix<float>{
                     {2.3107890}, {5.13921615}, {7.96764328}, {10.03819564}});
    REQUIRE(d == blaze::DynamicMatrix<float>{{0}, {0}, {0}, {-2.828427}});

    Signal1D composed;
    wb.Compose(&composed);

    REQUIRE(data == composed);
  }

  SECTION("Signal size = 16, db4") {
    Signal1D data = {-2, -4, -4, 4, 2, -1, 5, -4, 0, -1, 3, 3, -3, 0, 0, 2};

    WaveletParameters wp = MakeParams({data.size()}, 1, WaveletTypes::kDB4);

    WaveletBuffer wb(wp);

    wb.Decompose(data, denoiser);

    const auto a = wb.decompositions()[0][1];
    const auto d = wb.decompositions()[0][0];

    REQUIRE(a == blaze::DynamicMatrix<float>{{-6.153706},
                                             {2.179678},
                                             {3.048263},
                                             {-2.246743},
                                             {1.633754},
                                             {0.981337},
                                             {-0.670319},
                                             {1.227735}});
    REQUIRE(d == blaze::DynamicMatrix<float>{{5.848158},
                                             {2.399488},
                                             {1.5020429},
                                             {-4.2296231},
                                             {0.3097073},
                                             {-1.9929088},
                                             {-0.7955451},
                                             {-1.6271051}});

    Signal1D composed;
    wb.Compose(&composed);

    REQUIRE(data == composed);
  }
}

TEST_CASE("Constant amplitude for all scales", "[wavelets]") {
  NullDenoiseAlgorithm<float> denoiser;

  Signal1D data(24);
  data = 6.4;

  WaveletBuffer wb(MakeParams({data.size()}, 3));
  wb.Decompose(data, denoiser);

  auto scale = GENERATE(0, 1, 2, 3);
  Signal1D composed;
  wb.Compose(&composed, scale);

  REQUIRE(blaze::mean(data) == Catch::Approx(blaze::mean(composed)));
}

TEST_CASE("Constant amplitude for ND Matrix", "[NDamplitutde]") {
  NullDenoiseAlgorithm<float> denoiser;
  DataGenerator dg;

  auto buffer_num = GENERATE(0, 1);
  CAPTURE(buffer_num);

  auto scale = GENERATE(0, 1, 2, 3);
  CAPTURE(scale);

  std::vector buffers = {WaveletBuffer(MakeParams({100}, scale)),
                         WaveletBuffer(MakeParams({100, 100}, scale))};
  std::vector signals = {SignalN2D{dg.GenerateMatrix2d(100, 1)},
                         SignalN2D{dg.GenerateMatrix2d(100, 100)}};
  REQUIRE(Decompose(&buffers[buffer_num], signals[buffer_num]));

  auto output_signal = SignalN2D{};
  Compose(buffers[buffer_num], &output_signal, scale);

  REQUIRE_THAT(blaze::mean(blaze::abs(output_signal[0])),
               Catch::Matchers::WithinAbs(
                   blaze::mean(blaze::abs(signals[buffer_num][0])), 0.01));
}

// TODO(victor1234): for future wavelet work
/*
TEST_CASE("sinus", "[wavelets]") {
  NullDenoiseAlgorithm<float> denoiser;

  Signal1D data(256);
  for (size_t i = 0; i < data.size(); ++i) {
    data[i] = sin(0.75 * 2 * 3.14 * i / data.size());
    //data[i] = sin((i + 190) / 100.) + 1;
  }

  WaveletBuffer wb(MakeParams({data.size()}, 4, WaveletTypes::kDB5));
  wb.Decompose(data, denoiser);
  std::cout << wb << std::endl;
}
*/
