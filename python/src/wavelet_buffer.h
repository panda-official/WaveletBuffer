// Copyright 2022 PANDA GmbH

#ifndef PYTHON_SRC_WAVELET_BUFFER_H_
#define PYTHON_SRC_WAVELET_BUFFER_H_

#include <pybind11/pybind11.h>

namespace py = pybind11;

void WrapWaveletBuffer(py::module *m);

#endif  // PYTHON_SRC_WAVELET_BUFFER_H_
