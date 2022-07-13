// Copyright 2020-2022 PANDA GmbH

#ifndef PYTHON_SRC_COMMON_H_
#define PYTHON_SRC_COMMON_H_

#include <wavelet_buffer/wavelet_parameters.h>

std::string WaveletTypeRepr(drift::WaveletTypes value);

std::string WaveletParametersRepr(const std::string &class_name,
                                  const drift::WaveletParameters &params);

#endif  // PYTHON_SRC_COMMON_H_
