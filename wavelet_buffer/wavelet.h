// Copyright 2022 PANDA GmbH

#ifndef WAVELET_BUFFER_METRIC_H_
#define WAVELET_BUFFER_METRIC_H_

#include <blaze/Blaze.h>
#include <tuple>

#include "wavelet_buffer/primitives.h"

namespace drift::dsp::wavelet {
/**
 * Padding type
 */
enum class Padding { ZeroDerivative, Periodized };

/**
 * Construct convolutional matrix for wavelet transform
 * @param size
 * @param order
 * @param padding padding type
 * @return
 */
blaze::CompressedMatrix<DataType> DaubechiesMat(size_t size, int order = 4,
                                         Padding padding = Padding::Periodized);

Signal2D dwt2s(Signal2D const &x, Signal2DCompressed const &dmat_w,
               Signal2DCompressed const &dmat_h);

std::tuple<Signal2D, Signal2D, Signal2D, Signal2D> dwt2(
    Signal2D const &x, Signal2DCompressed const &dmat_w,
    Signal2DCompressed const &dmat_h);

Signal2D idwt2(const Signal2D &ll, const Signal2D &lh, const Signal2D &hl,
               const Signal2D &hh, const Signal2DCompressed &dmat_w,
               const Signal2DCompressed &dmat_h);

Signal1D dbwavf(const int wnum);

// orthfilt
std::tuple<Signal1D, Signal1D, Signal1D, Signal1D> Orthfilt(
    Signal1D const &W_in);


}  // namespace drift::dsp::wavelet

#endif  // WAVELET_BUFFER_METRIC_H_
