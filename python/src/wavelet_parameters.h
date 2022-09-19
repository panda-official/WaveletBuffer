// Copyright 2020-2022 PANDA GmbH

#ifndef PYTHON_SRC_WAVELET_PARAMETERS_H_
#define PYTHON_SRC_WAVELET_PARAMETERS_H_

#include <pybind11/pybind11.h>
#include <wavelet_buffer/wavelet_parameters.h>

#include <string>

namespace py = pybind11;

std::string WaveletTypeRepr(drift::WaveletTypes value);
std::string WaveletParametersRepr(const std::string &class_name,
                                  const drift::WaveletParameters &params);
void WrapWaveletType(py::module *m);
void WrapWaveletParameters(py::module *m);

#endif  // PYTHON_SRC_WAVELET_PARAMETERS_H_
