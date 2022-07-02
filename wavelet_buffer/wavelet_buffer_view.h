// Copyright 2021 PANDA GmbH

#ifndef WAVELET_BUFFER_WAVELET_BUFFER_VIEW_H_
#define WAVELET_BUFFER_WAVELET_BUFFER_VIEW_H_

#include <memory>

#include "wavelet_buffer/wavelet_buffer.h"

namespace drift {

/**
 * @class WaveletBufferView
 *
 * View of wavelet buffer to compose and decompose a part of the signal in the
 * decomposition
 */
class WaveletBufferView {
  friend class WaveletBuffer;

 public:
  using NWaveletDecompositionView = blaze::Subvector<NWaveletDecomposition>;

  ~WaveletBufferView();

  /**
   * Decomposes the signal into the subbands and saves it internally
   * @param data the signal
   * @return true if it has no errors
   */
  bool Decompose(const SignalN2D& data,
                 const DenoiseAlgorithm<DataType>& denoiser);

  /**
   * Composes the internal subbands into a signal
   * @param data the signal
   * @param scale_factor wavelet scale factor 0 - all the steps
   * of the decomposition recomposed and the output has size of the original
   * signal. If factor N - all the step - N are recomposed and output has size
   * 2^N smaller
   * @return true if it has no errors
   */
  bool Compose(SignalN2D* data, int scale_factor = 0) const;

  [[nodiscard]] NWaveletDecompositionView decompositions() const;

  /**
   * Cast to a new wavelet buffer
   * @return
   */
  operator WaveletBuffer() const;

 private:
  WaveletBufferView(int start_signal, int count, WaveletBuffer*);
  WaveletBufferView(int start_signal, int count, const WaveletBuffer*);

  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace drift

#endif  // WAVELET_BUFFER_WAVELET_BUFFER_VIEW_H_
