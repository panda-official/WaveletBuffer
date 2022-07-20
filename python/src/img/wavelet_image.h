// Copyright 2022 PANDA GmbH

#ifndef SRC_WAVELET_IMAGE_H_
#define SRC_WAVELET_IMAGE_H_

#include <pybind11/pybind11.h>

namespace py = pybind11;

void WrapWaveletImage(py::module *m);

#endif  // SRC_WAVELET_IMAGE_H_
