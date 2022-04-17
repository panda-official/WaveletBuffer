// Copyright 2022 PANDA GmbH

#ifndef DRIFT_UTILS_INIT_H_
#define DRIFT_UTILS_INIT_H_

#include <wavelet_buffer/primitives.h>

namespace drift::utils {

drift::dsp::Signal1D GetRandomSignal(size_t length);
drift::dsp::SignalN2D GetRandomSignal(size_t rows, size_t columns);

}  // namespace drift::utils

#endif  // DRIFT_UTILS_INIT_H_
