// Copyright 2021-2022 PANDA GmbH

#ifndef PYTHON_SRC_IMG_CODECS_H_
#define PYTHON_SRC_IMG_CODECS_H_

#include <pybind11/pybind11.h>
#include <wavelet_buffer/img/image_codec.h>

namespace py = pybind11;

void WrapCodecAlgorithms(py::module* m);

#endif  // PYTHON_SRC_IMG_CODECS_H_
