// Copyright 2021 PANDA GmbH

#ifndef SRC_DSP_WAVELET_BUFFER_VIEW_PROXY_H_
#define SRC_DSP_WAVELET_BUFFER_VIEW_PROXY_H_

#include <pybind11/pytypes.h>

struct WaveletBufferViewProxy {
  WaveletBufferViewProxy(const drift::WaveletBuffer& buffer,
                         int start_signal, int count)
      : buffer(pybind11::cast(buffer)),
        start_signal(start_signal),
        count(count) {}

  pybind11::object buffer;
  int start_signal;
  int count;
};

#endif  // SRC_DSP_WAVELET_BUFFER_VIEW_PROXY_H_
