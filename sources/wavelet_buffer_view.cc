// Copyright 2021 PANDA GmbH
#include "wavelet_buffer/wavelet_buffer_view.h"

#include <memory>
#include <iostream>

#include "wavelet_buffer/wavelet_utils.h"

namespace drift {

class WaveletBufferView::Impl {
 public:
  Impl(int start_signal, int count, const WaveletBuffer* buffer)
      : start_signal_(start_signal),
        count_(count),
        buffer_(nullptr),
        const_buffer_(buffer) {}

  Impl(int start_signal, int count, WaveletBuffer* buffer)
      : start_signal_(start_signal),
        count_(count),
        buffer_(buffer),
        const_buffer_(buffer) {}

  bool Decompose(const SignalN2D& data,
                 const DenoiseAlgorithm<DataType>& denoiser) {
    auto ret = CheckChannelRange();
    return ret && internal::DecomposeImpl(buffer_->parameters(),
                                          &buffer_->decompositions(), data,
                                          denoiser, start_signal_, count_);
  }

  bool Compose(SignalN2D* data, int scale_factor) const {
    auto ret = CheckChannelRange();
    return ret && internal::ComposeImpl(const_buffer_->parameters(), data,
                                        const_buffer_->decompositions(),
                                        scale_factor, start_signal_, count_);
  }

  NWaveletDecompositionView decompositions() {
    return blaze::subvector(buffer_->decompositions(), start_signal_, count_);
  }

  [[nodiscard]] WaveletBuffer CastToBuffer() const {
    auto new_buf_params = const_buffer_->parameters();
    new_buf_params.signal_number = count_;

    WaveletBuffer new_buffer(new_buf_params);
    for (int ch = 0; ch < count_; ++ch) {
      new_buffer.decompositions()[ch] =
          const_buffer_->decompositions()[ch + start_signal_];
    }

    return new_buffer;
  }

 private:
  bool CheckChannelRange() const {
    if (const_buffer_->parameters().signal_number < start_signal_ + count_) {
      std::cerr << "View out of range of the buffer's channels";
      return false;
    }
    return true;
  }

  int start_signal_;
  int count_;
  WaveletBuffer* buffer_;
  const WaveletBuffer* const_buffer_;
};

WaveletBufferView::~WaveletBufferView() = default;

bool WaveletBufferView::Decompose(const SignalN2D& data,
                                  const DenoiseAlgorithm<DataType>& denoiser) {
  return impl_->Decompose(data, denoiser);
}

bool WaveletBufferView::Compose(SignalN2D* data, int scale_factor) const {
  return impl_->Compose(data, scale_factor);
}

WaveletBufferView::NWaveletDecompositionView WaveletBufferView::decompositions()
    const {
  return impl_->decompositions();
}

WaveletBufferView::WaveletBufferView(int start_signal, int count,
                                     WaveletBuffer* buffer)
    : impl_(std::make_unique<Impl>(start_signal, count, buffer)) {}

WaveletBufferView::WaveletBufferView(int start_signal, int count,
                                     const WaveletBuffer* buffer)
    : impl_(std::make_unique<Impl>(start_signal, count, buffer)) {}

WaveletBufferView::operator WaveletBuffer() const {
  return impl_->CastToBuffer();
}

}  // namespace drift
