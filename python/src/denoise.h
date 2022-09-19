// Copyright 2020-2022 PANDA GmbH

#ifndef PYTHON_SRC_DENOISE_H_
#define PYTHON_SRC_DENOISE_H_

#include <pybind11/pybind11.h>

namespace py = pybind11;

void WrapDenoiseAlgorithms(py::module *m);

#endif  // PYTHON_SRC_DENOISE_H_
