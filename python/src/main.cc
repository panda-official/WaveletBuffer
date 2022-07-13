// Copyright 2020-2022 PANDA GmbH

#include "denoise.h"
#include "wavelet_buffer.h"
#include "wavelet_buffer_view.h"
#include "wavelet_image.h"
#include "wavelet_parameters.h"


PYBIND11_MODULE(_wavelet_buffer, m) {
  WrapWaveletType(&m);
  WrapWaveletParameters(&m);
  WrapDenoiseAlgorithms(&m);
  WrapWaveletBuffer(&m);
  WrapWaveletViewBuffer(&m);
  WrapWaveletImage(&m);
}
