// Copyright 2020-2021 PANDA GmbH

#include <wavelet_buffer/wavelet_buffer.h>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "blaze_utils.h"
#include "wavelet_buffer_view_proxy.h"

namespace py = pybind11;

using drift::DecompositionSize;
using drift::Distance;
using drift::EnergyDistribution;
using drift::Signal1D;
using drift::Signal2D;
using drift::SignalN2D;
using drift::WaveletBuffer;
using drift::WaveletParameters;
using drift::WaveletTypes;
using Denoiser = drift::DenoiseAlgorithm<float>;


std::string WaveletParametersRepr(const std::string &class_name,
                                  const WaveletParameters &params);

/**
 *
 * @param self
 * @param data
 * @param denoiser
 */
void DecomposeN2DSignal(WaveletBuffer *self, const py::array &data,
                        const Denoiser &denoiser) {
  const auto &params = self->parameters();

  if (data.ndim() != 3) {
    throw py::buffer_error("input data must have 3 dim (actual " +
                           std::to_string(data.ndim()) + ")");
  }

  if (data.shape()[0] != params.signal_number) {
    throw py::buffer_error(
        "input data must have " + std::to_string(params.signal_number) +
        " channels (actual " + std::to_string(data.shape()[0]) + ")");
  }

  if (data.shape()[1] != params.signal_shape[1]) {
    throw py::buffer_error(
        "input data must have " + std::to_string(params.signal_shape[1]) +
        " height (actual " + std::to_string(data.shape()[1]) + ")");
  }

  if (data.shape()[2] != params.signal_shape[0]) {
    throw py::buffer_error(
        "input data must have " + std::to_string(params.signal_shape[0]) +
        " width (actual " + std::to_string(data.shape()[2]) + ")");
  }

  auto signal = PyArrayToSignalN2D(data);
  if (!self->Decompose(signal, denoiser)) {
    throw py::buffer_error("Failed to decompose data");
  }
}

/**
 *
 * @param self
 * @param data
 * @param denoiser
 */
void Decompose1DSignal(WaveletBuffer *self, const py::array &data,
                       const Denoiser &denoiser) {
  const auto &params = self->parameters();

  if (data.ndim() != 1) {
    throw py::buffer_error("input data must have 1 dim (actual " +
                           std::to_string(data.ndim()) + ")");
  }

  if (data.shape()[0] != params.signal_shape[0]) {
    throw py::buffer_error(
        "input data must be " + std::to_string(params.signal_shape[0]) +
        " items long (actual " + std::to_string(data.shape()[0]) + ")");
  }

  Signal1D signal(params.signal_shape[0]);
  int i = 0;
  for (auto &m : data) {
    signal[i++] = m.cast<float>();
  }

  if (!self->Decompose(signal, denoiser)) {
    throw py::buffer_error("Failed to decompose data");
  }
}

/**
 *
 * @param self
 * @return
 */
py::array_t<float> Compose1DSignal(const WaveletBuffer &self,
                                   int scale_factor) {
  Signal1D data;
  if (!self.Compose(&data, scale_factor)) {
    throw py::buffer_error("Failed to compose data");
  }

  return py::array_t<float>(data.size(), data.data());
}

/**
 *
 * @param self
 * @return
 */
py::array_t<float> ComposeN2DSignal(const WaveletBuffer &self,
                                    int scale_factor) {
  SignalN2D data;
  if (!self.Compose(&data, scale_factor)) {
    throw py::buffer_error("Failed to compose data");
  }

  return SignalN2DToPyArray(data);
}

void WrapWaveletBuffer(py::module *m) {
  using Class = drift::WaveletBuffer;

  m->def("distance",
         [](const Class &a, const Class &b) { return Distance(a, b); });

  m->def("energy_distribution", [](const Class &buffer) -> py::array_t<float> {
    auto dist = EnergyDistribution(buffer);
    if (buffer.parameters().dimension() == 1) {
      return py::array_t<float>(dist[0].size(), dist[0].data());
    } else {
      return VecVecToPyArray(dist);
    }
  });

  auto cls = py::class_<Class>(*m, "WaveletBuffer");
  cls.def(py::init([](const std::vector<size_t> &signal_shape,
                      size_t channel_number, size_t decomposition_steps,
                      WaveletTypes wavelet_type,
                      const NPyDecomposition &decompositions) {
            WaveletParameters parameters{
                .signal_shape = signal_shape,
                .signal_number = channel_number,
                .decomposition_steps = decomposition_steps,
                .wavelet_type = wavelet_type,
            };
            if (decompositions.empty()) {
              return std::make_unique<Class>(parameters);
            } else {
              return std::make_unique<Class>(
                  parameters, NPyDecompositionToNW(decompositions));
            }
          }),
          py::arg("signal_shape"), py::arg("signal_number"),
          py::arg("decomposition_steps"), py::arg("wavelet_type"),
          py::arg("decompositions") = NPyDecomposition());

  cls.def_static(
      "decomposition_size",
      [](const std::vector<size_t> &signal_shape, size_t channel_number,
         size_t decomposition_steps, WaveletTypes wavelet_type) {
        return DecompositionSize(WaveletParameters{
            .signal_shape = signal_shape,
            .signal_number = channel_number,
            .decomposition_steps = decomposition_steps,
            .wavelet_type = wavelet_type,
        });
      },
      py::arg("signal_shape"), py::arg("signal_number"),
      py::arg("decomposition_steps"), py::arg("wavelet_type"));

  cls.def_static("parse", &Class::Parse, py::arg("blob"));

  cls.def(
      "decompose",
      [](Class &self, const py::array &data, const Denoiser &denoiser) {
        const auto &params = self.parameters();
        if (params.dimension() == 1) {
          Decompose1DSignal(&self, data, denoiser);
        } else if (params.dimension() == 2) {
          DecomposeN2DSignal(&self, data, denoiser);
        } else {
          throw py::buffer_error("Unsupported number of dimensions");
        }
      },
      py::arg("data"), py::arg("denoiser"));

  cls.def(
      "compose",
      [](const Class &self, int scale_factor) -> py::array_t<float> {
        const auto &params = self.parameters();

        if (params.dimension() == 1) {
          return Compose1DSignal(self, scale_factor);
        } else if (params.dimension() == 2) {
          return ComposeN2DSignal(self, scale_factor);
        }
        throw py::buffer_error("Unsupported number of dimensions");
      },
      py::arg("scale_factor") = 0);

  cls.def(
      "serialize",
      [](const Class &self, int compression_level) {
        std::string blob;
        if (!self.Serialize(&blob, compression_level)) {
          throw py::value_error("Failed to serialize buffer");
        }

        return py::bytes(blob);
      },
      py::arg("compression_level") = 0);

  cls.def("__eq__",
          [](const Class &a, const Class &b) -> bool { return a == b; });

  cls.def_property_readonly("decompositions", [](const Class &self) {
    return NWaveletDecompositionToNPy(self.decompositions());
  });

  cls.def(
      "__getitem__",
      [](const Class &self, const py::slice &slice) {
        const auto &params = self.parameters();
        size_t start, stop, step, slicelength;
        if (!slice.compute(params.signal_number, &start, &stop, &step,
                           &slicelength)) {
          throw py::error_already_set();
        }

        if (step > 1) {
          throw py::value_error("Step > 1 is not supported.");
        }

        if (start == stop) {
          throw py::value_error("Empty slice");
        }

        return std::make_unique<WaveletBufferViewProxy>(self, start,
                                                        stop - start);
      },
      py::arg("slice"));

  cls.def_property_readonly("parameters", &Class::parameters);

  cls.def("get_value_range", &Class::GetValueRange, py::arg("index"));

  cls.def("is_empty", [](const Class &self) -> bool { return self.IsEmpty(); });

  cls.def("__repr__", [](Class &self) {
    return WaveletParametersRepr("WaveletBuffer", self.parameters());
  });
}
