// Copyright 2021 PANDA GmbH
#ifndef WAVELET_BUFFER_PRIMITIVES_H_
#define WAVELET_BUFFER_PRIMITIVES_H_

#include <blaze/Blaze.h>

#include <algorithm>
#include <cmath>
#include <ostream>

namespace drift::dsp {

using DataType = float;
using Signal1D = blaze::DynamicVector<DataType>;
using Signal1DCompressed = blaze::CompressedVector<DataType>;
using Signal2D = blaze::DynamicMatrix<DataType>;
using Signal2DCompressed = blaze::CompressedMatrix<DataType>;
using SignalN2D = blaze::DynamicVector<Signal2D>;

struct Size {
  int width;
  int height;
};

}  // namespace drift::dsp

#endif  // WAVELET_BUFFER_PRIMITIVES_H_
