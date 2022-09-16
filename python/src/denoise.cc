// Copyright 2020-2022 PANDA GmbH

#include "src/denoise.h"

#include <wavelet_buffer/denoise_algorithms.h>

void WrapDenoiseAlgorithms(py::module *m) {
  auto module = m->def_submodule("denoise");

  auto base = py::class_<drift::DenoiseAlgorithm<float>>(module, "Base");

  py::class_<drift::NullDenoiseAlgorithm<float>,
             drift::DenoiseAlgorithm<float>>(module, "Null")
      .def(py::init());

  py::class_<drift::SimpleDenoiseAlgorithm<float>,
             drift::DenoiseAlgorithm<float>>(module, "Simple")
      .def(py::init<float>(), py::arg("compression_level"));

  py::class_<drift::ThresholdAbsDenoiseAlgorithm<float>,
             drift::DenoiseAlgorithm<float>>(module, "Threshold")
      .def(py::init<float, float>(), py::arg("a"), py::arg("b"));
}
