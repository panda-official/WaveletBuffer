// Copyright 2021 PANDA GmbH

#include <wavelet_buffer/wavelet_buffer_view.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include "blaze_utils.h"

namespace py = pybind11;

using drift::WaveletBuffer;
using Denoiser = drift::DenoiseAlgorithm<float>;
using drift::Signal1D;
using drift::Signal2D;
using drift::SignalN2D;

#include "wavelet_buffer_view_proxy.h"

/**
 *
 * @param self
 * @param data
 * @param denoiser
 */
void DecomposeN2DSignal(WaveletBufferViewProxy *self, const py::array &data,
                        const Denoiser &denoiser) {
  auto &buffer = self->buffer.cast<WaveletBuffer &>();
  const auto &params = buffer.parameters();

  if (data.ndim() != 3) {
    throw py::buffer_error("Input data must have 3 dim (actual " +
                           std::to_string(data.ndim()) + ")");
  }

  if (data.shape()[0] != self->count) {
    throw py::buffer_error("Input data must have " +
                           std::to_string(self->count) + " channels (actual " +
                           std::to_string(data.shape()[0]) + ")");
  }

  if (data.shape()[1] != params.signal_shape[1]) {
    throw py::buffer_error(
        "Input data must have " + std::to_string(params.signal_shape[1]) +
        " height (actual " + std::to_string(data.shape()[1]) + ")");
  }

  if (data.shape()[2] != params.signal_shape[0]) {
    throw py::buffer_error(
        "Input data must have " + std::to_string(params.signal_shape[0]) +
        " width (actual " + std::to_string(data.shape()[2]) + ")");
  }

  auto signal = PyArrayToSignalN2D(data);
  if (!buffer(self->start_signal, self->count).Decompose(signal, denoiser)) {
    throw py::buffer_error("Failed to decompose data");
  }
}

py::array_t<float> ComposeN2DSignal(WaveletBufferViewProxy *self,
                                    int scale_factor) {
  SignalN2D data;
  auto &buffer = self->buffer.cast<WaveletBuffer &>();
  if (!buffer(self->start_signal, self->count).Compose(&data, scale_factor)) {
    throw py::buffer_error("Failed to compose data");
  }

  return SignalN2DToPyArray(data);
}

void WrapWaveletViewBuffer(py::module *m) {
  auto cls = py::class_<WaveletBufferViewProxy>(*m, "WaveletBufferView");
  cls.def(
      "decompose",
      [](WaveletBufferViewProxy &self, const py::array &data,
         const Denoiser &denoiser) {
        const auto &params = self.buffer.cast<WaveletBuffer &>().parameters();
        if (params.dimension() != 2) {
          throw py::buffer_error("Unsupported number of dimensions");
        }

        DecomposeN2DSignal(&self, data, denoiser);
      },
      py::arg("data"), py::arg("denoiser"));
  cls.def(
      "compose",
      [](WaveletBufferViewProxy &self, int scale_factor) {
        const auto &params = self.buffer.cast<WaveletBuffer &>().parameters();
        if (params.dimension() != 2) {
          throw py::buffer_error("Unsupported number of dimensions");
        }
        return ComposeN2DSignal(&self, scale_factor);
      },
      py::arg("scale_factor") = 0);
}
