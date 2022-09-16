// Copyright 2022 PANDA GmbH

#ifndef PYTHON_SRC_IMG_WAVELET_IMAGE_H_
#define PYTHON_SRC_IMG_WAVELET_IMAGE_H_

#include <pybind11/pybind11.h>

namespace py = pybind11;

void WrapWaveletImage(py::module *m);

#endif  // PYTHON_SRC_IMG_WAVELET_IMAGE_H_
