// Copyright 2020-2022 PANDA GmbH

#include "wavelet_parameters.h"
#include "denoise.h"

namespace py = pybind11;

void WrapWaveletBuffer(py::module *m);
void WrapWaveletViewBuffer(py::module *m);
void WrapWaveletImage(py::module *m);

PYBIND11_MODULE(_wavelet_buffer, m) {
  WrapWaveletType(&m);
  WrapWaveletParameters(&m);
  WrapDenoiseAlgorithms(&m);
  WrapWaveletBuffer(&m);
  WrapWaveletViewBuffer(&m);
  WrapWaveletImage(&m);
}
