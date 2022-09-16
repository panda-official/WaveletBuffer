// Copyright 2020-2022 PANDA GmbH

#include "src/denoise.h"
#include "src/img/codecs.h"
#include "src/img/wavelet_image.h"
#include "src/wavelet_buffer.h"
#include "src/wavelet_buffer_view.h"
#include "src/wavelet_parameters.h"

PYBIND11_MODULE(_wavelet_buffer, m) {
  WrapWaveletType(&m);
  WrapWaveletParameters(&m);
  WrapDenoiseAlgorithms(&m);
  WrapWaveletBuffer(&m);
  WrapWaveletViewBuffer(&m);
  WrapCodecAlgorithms(&m);
  WrapWaveletImage(&m);
}
