// Copyright 2021 PANDA GmbH
#ifndef DRIFT_DSP_PRIMITIVES_H_
#define DRIFT_DSP_PRIMITIVES_H_

#include <blaze/Blaze.h>

#include <algorithm>
#include <cmath>
#include <ostream>

namespace drift::dsp {

using DataType = float;
using Signal1D = blaze::DynamicVector<DataType>;
using Signal2D = blaze::DynamicMatrix<DataType>;
using SignalN2D = blaze::DynamicVector<Signal2D>;

struct Size {
  int width;
  int height;
};

}  // namespace drift::dsp

#endif  // DRIFT_DSP_PRIMITIVES_H_
