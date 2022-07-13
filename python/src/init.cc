// Copyright 2020 PANDA GmbH
#include <pybind11/pybind11.h>

#include "common.h"

namespace py = pybind11;

void WrapEnums(py::module *m);
void WrapWaveletParameters(py::module *m);
void WrapDenoiseAlgorithms(py::module *m);
void WrapWaveletBuffer(py::module *m);
void WrapWaveletViewBuffer(py::module *m);
void WrapWaveletImage(py::module *m);

PYBIND11_MODULE(_wavelet_buffer, m) {
  WrapEnums(&m);
  WrapWaveletParameters(&m);
  WrapDenoiseAlgorithms(&m);
  WrapWaveletBuffer(&m);
  WrapWaveletViewBuffer(&m);
  WrapWaveletImage(&m);
}
