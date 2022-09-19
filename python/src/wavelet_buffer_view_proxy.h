// Copyright 2021-2022 PANDA GmbH

#ifndef PYTHON_SRC_WAVELET_BUFFER_VIEW_PROXY_H_
#define PYTHON_SRC_WAVELET_BUFFER_VIEW_PROXY_H_

#include <pybind11/pytypes.h>
#include <wavelet_buffer/wavelet_buffer.h>

struct WaveletBufferViewProxy {
  WaveletBufferViewProxy(const drift::WaveletBuffer& buffer, int start_signal,
                         int count)
      : buffer(pybind11::cast(buffer)),
        start_signal(start_signal),
        count(count) {}

  pybind11::object buffer;
  int start_signal;
  int count;
};

#endif  // PYTHON_SRC_WAVELET_BUFFER_VIEW_PROXY_H_
