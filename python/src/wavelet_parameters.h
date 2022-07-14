// Copyright 2020-2022 PANDA GmbH

#ifndef SRC_WAVELET_TYPE_H_
#define SRC_WAVELET_TYPE_H_

#include <pybind11/pybind11.h>
#include <wavelet_buffer/wavelet_parameters.h>

namespace py = pybind11;

std::string WaveletTypeRepr(drift::WaveletTypes value);
std::string WaveletParametersRepr(const std::string &class_name,
                                  const drift::WaveletParameters &params);
void WrapWaveletType(py::module *m);
void WrapWaveletParameters(py::module *m);

#endif  // SRC_WAVELET_TYPE_H_
