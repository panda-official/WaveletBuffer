// Copyright 2020 PANDA GmbH
#include <pybind11/pybind11.h>

namespace py = pybind11;

void WrapEnums(py::module *m);
void WrapDenoiseAlgorithms(py::module *m);
void WrapWaveletBuffer(py::module *m);
//extern void WrapWaveletViewBuffer(py::module *m);

PYBIND11_MODULE(_wavelet_buffer, m) {
  WrapEnums(&m);
  WrapDenoiseAlgorithms(&m);
  WrapWaveletBuffer(&m);
  //WrapWaveletViewBuffer(&m);
}
