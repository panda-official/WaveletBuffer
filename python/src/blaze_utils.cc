// Copyright 2021 PANDA GmbH
#include "blaze_utils.h"

#include <utility>

namespace py = pybind11;
using drift::NWaveletDecomposition;
using drift::Signal2D;
using drift::SignalN2D;

SignalN2D PyArrayToSignalN2D(const py::array &data) {
  auto shape = data.shape();
  SignalN2D signal(
      shape[0], {static_cast<size_t>(shape[1]), static_cast<size_t>(shape[2])});
  std::vector<size_t> strides = {static_cast<size_t>(shape[1] * shape[2]),
                                 static_cast<size_t>(shape[2]), 1};

  auto ptr = static_cast<const float *>(data.request(true).ptr);
  for (int ch = 0; ch < shape[0]; ++ch) {
    const auto ch_offset = strides[0] * ch;
    for (int i = 0; i < shape[1]; ++i) {
      const auto raw_offset = ch_offset + strides[1] * i;
      for (int j = 0; j < shape[2]; ++j) {
        signal[ch](i, j) = ptr[raw_offset + j];
      }
    }
  }

  return signal;
}

py::array_t<float> SignalN2DToPyArray(const SignalN2D &data) {
  std::vector<size_t> shape = {
      data.size(),
      data[0].rows(),
      data[0].columns(),
  };

  std::vector<size_t> el_strides = {shape[1] * shape[2], shape[2], 1};

  constexpr size_t kElSize = sizeof(float);
  std::vector<size_t> strides = {kElSize * el_strides[0],
                                 kElSize * el_strides[1],
                                 kElSize * el_strides[2]};

  py::array_t<float> result(shape, strides);
  auto ptr = static_cast<float *>(result.request(true).ptr);
  for (int ch = 0; ch < shape[0]; ++ch) {
    const auto ch_offset = el_strides[0] * ch;
    for (int i = 0; i < shape[1]; ++i) {
      const auto raw_offset = ch_offset + el_strides[1] * i;
      for (int j = 0; j < shape[2]; ++j) {
        ptr[raw_offset + j] = data[ch](i, j);
      }
    }
  }

  return result;
}

NPyDecomposition NWaveletDecompositionToNPy(
    const NWaveletDecomposition &decompositions) {
  NPyDecomposition py_decompositions(decompositions.size());

  for (auto ch = 0; ch < decompositions.size(); ++ch) {
    auto &channel = decompositions[ch];
    std::vector<py::array_t<float>> subbands(channel.size());

    for (int num = 0; num < subbands.size(); ++num) {
      std::vector<size_t> shape = {
          channel[num].rows(),
          channel[num].columns(),
      };

      std::vector<size_t> el_strides = {
          shape[1],
          1,
      };

      constexpr size_t kElSize = sizeof(float);
      std::vector<size_t> strides = {
          kElSize * el_strides[0],
          kElSize * el_strides[1],
      };

      py::array_t<float> subband(shape, strides);
      auto ptr = static_cast<float *>(subband.request(true).ptr);
      std::memset(ptr, 0, shape[0] * shape[1] * kElSize);
      for (int i = 0; i < shape[0]; ++i) {
        for (int j = 0; j < shape[1]; ++j) {
          ptr[el_strides[0] * i + j] = channel[num].at(i, j);
        }
      }

      subbands[num] = subband;
    }

    py_decompositions[ch] = subbands;
  }

  return py_decompositions;
}

NWaveletDecomposition NPyDecompositionToNW(
    const NPyDecomposition &decompositions) {
  NWaveletDecomposition nw_decompositions(decompositions.size());
  for (int ch = 0; ch < decompositions.size(); ++ch) {
    nw_decompositions[ch].resize(decompositions[ch].size());

    for (int num = 0; num < nw_decompositions[ch].size(); ++num) {
      const auto signal = decompositions[ch][num];
      const auto shape = signal.shape();
      std::vector<ssize_t> el_strides = {shape[1], 1};

      blaze::DynamicMatrix<drift::DataType> subband(shape[0], shape[1]);
      auto ptr = static_cast<float *>(signal.request(true).ptr);
      for (int i = 0; i < shape[0]; ++i) {
        for (int j = 0; j < shape[1]; ++j) {
          subband(i, j) = ptr[el_strides[0] * i + j];
        }
      }

      nw_decompositions[ch][num] = subband;
    }
  }

  return nw_decompositions;
}

py::array_t<float> VecVecToPyArray(
    const blaze::DynamicVector<blaze::DynamicVector<float>> &data) {
  std::vector<size_t> shape = {
      data.size(),
      data[0].size(),
  };

  std::vector<size_t> el_strides = {shape[1], 1};

  constexpr size_t kElSize = sizeof(float);
  std::vector<size_t> strides = {kElSize * el_strides[0],
                                 kElSize * el_strides[1]};

  py::array_t<float> result(shape, strides);
  auto ptr = static_cast<float *>(result.request(true).ptr);
  for (int ch = 0; ch < shape[0]; ++ch) {
    for (int i = 0; i < shape[1]; ++i) {
      ptr[el_strides[0] * ch + i] = data[ch][i];
    }
  }

  return result;
}
