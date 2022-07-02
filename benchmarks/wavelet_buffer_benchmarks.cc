// Copyright 2020-2022 PANDA GmbH

#include <wavelet_buffer/wavelet.h>
#include <wavelet_buffer/primitives.h>
#include <wavelet_buffer/wavelet_buffer.h>
#include <wavelet_buffer/wavelet_parameters.h>
#include <wavelet_buffer/wavelet_utils.h>
#include <wavelet_buffer/denoise_algorithms.h>

#include <fstream>

#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

#include "benchmarks/init.h"

using drift::DataType;
using drift::Signal1D;
using drift::SignalN2D;
using drift::WaveletBuffer;
using drift::wavelet::DaubechiesMat;
using drift::wavelet::dbwavf;
using drift::wavelet::Orthfilt;
using drift::utils::GetRandomSignal;

TEST_CASE("Wavelet algorithms benchmark 1D") {
  using drift::NullDenoiseAlgorithm;

  auto k = GENERATE(0.1, 1, 60);

  const auto length = static_cast<size_t>(k * 48000);

  drift::WaveletParameters parameters = {
      .signal_shape = {length},
      .signal_number = 1,
      .decomposition_steps = 9,
      .wavelet_type = drift::WaveletTypes::kDB3,
      .is_raw_convolve_1d = true};

  auto data_src = GetRandomSignal(length);

  WaveletBuffer buffer(parameters);
  BENCHMARK("Decompose " + std::to_string(length)) {
    buffer.Decompose(data_src, NullDenoiseAlgorithm<DataType>());
  };

  BENCHMARK("Compose " + std::to_string(length)) {
    Signal1D data_dst;
    buffer.Compose(&data_dst);
  };
}

TEST_CASE("Convolution of long 1D signal") {
  auto k = GENERATE(0.1, 1, 60);
  const size_t length = k * 48000;
  const auto signal = GetRandomSignal(length);

  const auto [lo_d, hi_d, lo_r, hi_r] = Orthfilt(dbwavf(3));
  blaze::CompressedMatrix<DataType> dmat(2, lo_d.size());
  blaze::row(dmat, 0) = blaze::trans(blaze::reverse(lo_d));
  blaze::row(dmat, 1) = blaze::trans(blaze::reverse(hi_d));
  BENCHMARK("Raw convolve") {
    auto result = drift::internal::dwt(signal, dmat);
    return result;
  };

  const blaze::CompressedMatrix<DataType> filter = DaubechiesMat(length, 6);

  BENCHMARK("Predefined matrix convolve") {
    blaze::DynamicVector<DataType> result = filter * signal;
    return result;
  };
}

TEST_CASE("Wavelet algorithms benchmark 2D") {
  using drift::NullDenoiseAlgorithm;
  using drift::SimpleDenoiseAlgorithm;

  drift::WaveletParameters parameters = {
      .signal_shape = {100, 100},
      .signal_number = 1,
      .decomposition_steps = 2,
      .wavelet_type = drift::WaveletTypes::kDB3};

  auto data_src_100 = GetRandomSignal(100, 100);

  SECTION("Wavelet type") {
    SECTION("kDB3") {
      parameters.wavelet_type = drift::kDB3;
      WaveletBuffer buffer(parameters);

      BENCHMARK("Decompose 100x100") {
        buffer.Decompose(data_src_100, NullDenoiseAlgorithm<DataType>());
      };

      BENCHMARK("Compose 100x100") {
        SignalN2D data_dst;
        buffer.Compose(&data_dst);
      };
    }

    SECTION("kDB4") {
      parameters.wavelet_type = drift::kDB4;
      WaveletBuffer buffer(parameters);

      BENCHMARK("Decompose 100x100") {
        buffer.Decompose(data_src_100, NullDenoiseAlgorithm<DataType>());
      };

      BENCHMARK("Compose 100x100") {
        SignalN2D data_dst;
        buffer.Compose(&data_dst);
      };
    }
  }

  SECTION("Wavelet step") {
    SECTION("2 steps") {
      parameters.decomposition_steps = 2;
      WaveletBuffer buffer(parameters);

      BENCHMARK("Decompose 100x100") {
        buffer.Decompose(data_src_100, NullDenoiseAlgorithm<DataType>());
      };

      BENCHMARK("Compose 100x100") {
        SignalN2D data_dst;
        buffer.Compose(&data_dst);
      };
    }

    SECTION("3 steps") {
      parameters.decomposition_steps = 3;
      WaveletBuffer buffer(parameters);

      BENCHMARK("Decompose 100x100") {
        buffer.Decompose(data_src_100, NullDenoiseAlgorithm<DataType>());
      };

      BENCHMARK("Compose 100x100") {
        SignalN2D data_dst;
        buffer.Compose(&data_dst);
      };
    }
    SECTION("4 steps") {
      parameters.decomposition_steps = 4;
      WaveletBuffer buffer(parameters);

      BENCHMARK("Decompose 100x100") {
        buffer.Decompose(data_src_100, NullDenoiseAlgorithm<DataType>());
      };

      BENCHMARK("Compose 100x100") {
        SignalN2D data_dst;
        buffer.Compose(&data_dst);
      };
    }
  }

  SECTION("Wavelet decomposition type") {
    WaveletBuffer buffer(parameters);
    BENCHMARK("Decompose 100x100") {
      buffer.Decompose(data_src_100, NullDenoiseAlgorithm<DataType>());
    };

    BENCHMARK("Compose 100x100") {
      SignalN2D data_dst;
      buffer.Compose(&data_dst);
    };
  }

  SECTION("Wavelet image size to 60~50 width subbands") {
    SECTION("800×600") {
      parameters.signal_shape = {800, 600};
      parameters.decomposition_steps = 5;

      WaveletBuffer buffer(parameters);
      auto data_src = GetRandomSignal(600, 800);

      BENCHMARK("Decompose") {
        buffer.Decompose(data_src, NullDenoiseAlgorithm<DataType>());
      };

      BENCHMARK("Compose") {
        SignalN2D data_dst;
        buffer.Compose(&data_dst);
      };
    }

    SECTION("1920×1080") {
      parameters.signal_shape = {1920, 1080};
      parameters.decomposition_steps = 5;
      WaveletBuffer buffer(parameters);
      auto data_src = GetRandomSignal(1080, 1920);

      BENCHMARK("Decompose") {
        buffer.Decompose(data_src, NullDenoiseAlgorithm<DataType>());
      };

      BENCHMARK("Compose") {
        SignalN2D data_dst;
        buffer.Compose(&data_dst);
      };
    }
  }

  SECTION("Serialization") {
    parameters.signal_shape = {800, 600};
    parameters.decomposition_steps = 5;
    WaveletBuffer buffer(parameters);
    auto data_src = GetRandomSignal(600, 800);

    uint8_t compression_level = GENERATE(0, 8, 16);
    std::string blob;

    buffer.Decompose(data_src, SimpleDenoiseAlgorithm<DataType>(0.7));
    REQUIRE(buffer.Serialize(&blob, compression_level));

    BENCHMARK("Parse 800x600 with 5 steps: compression_level " +
              std::to_string(compression_level)) {
      auto b = WaveletBuffer::Parse(blob);
      return b;
    };

    BENCHMARK("Serialize 800x600 with 5 steps: compression_level = " +
              std::to_string(compression_level)) {
      auto ret = buffer.Serialize(&blob, compression_level);
      return ret;
    };
  }
}
