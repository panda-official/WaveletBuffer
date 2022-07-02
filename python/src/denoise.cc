// Copyright 2020 PANDA GmbH
#include <wavelet_buffer/denoise_algorithms.h>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

namespace py = pybind11;

void WrapDenoiseAlgorithms(py::module *m) {
  using drift::NullDenoiseAlgorithm;
  using drift::SimpleDenoiseAlgorithm;
  using drift::ThresholdAbsDenoiseAlgorithm;
  using drift::DenoiseAlgorithm;

  auto module = m->def_submodule("denoise");

  auto base = py::class_<DenoiseAlgorithm<float>>(module, "Base");
  py::class_<NullDenoiseAlgorithm<float>, DenoiseAlgorithm<float>>
      (module, "Null")
      .def(py::init());

  py::class_<SimpleDenoiseAlgorithm<float>, DenoiseAlgorithm<float>>
      (module, "Simple")
      .def(py::init<float>(),
           py::arg("compression_level"));

  py::class_<ThresholdAbsDenoiseAlgorithm<float>, DenoiseAlgorithm<float>>
      (module, "Threshold")
      .def(py::init<float, float>(),
          py::arg("a"), py::arg("b"));
}
