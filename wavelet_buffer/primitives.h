// Copyright 2021 PANDA GmbH
#ifndef WAVELET_BUFFER_PRIMITIVES_H_
#define WAVELET_BUFFER_PRIMITIVES_H_

#include <blaze/Blaze.h>

#include <algorithm>
#include <cmath>
#include <ostream>

namespace drift {

using DataType = float;
using Signal1D = blaze::DynamicVector<DataType>;
using Signal2D = blaze::DynamicMatrix<DataType>;
using SignalN2D = blaze::DynamicVector<Signal2D>;

struct Size {
  int width;
  int height;
};

}  // namespace drift

#endif  // WAVELET_BUFFER_PRIMITIVES_H_
