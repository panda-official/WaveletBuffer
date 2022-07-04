// Copyright 2022 PANDA GmbH

#ifndef WAVELET_BUFFER_WAVELET_H_
#define WAVELET_BUFFER_WAVELET_H_

#include <blaze/Blaze.h>

#include <tuple>

#include "wavelet_buffer/primitives.h"

namespace drift::wavelet {

using Signal2DCompressed = blaze::CompressedMatrix<DataType>;

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
blaze::CompressedMatrix<DataType> DaubechiesMat(
    size_t size, int order = 4, Padding padding = Padding::Periodized);

Signal2D dwt2s(Signal2D const &x, Signal2DCompressed const &dmat_w,
               Signal2DCompressed const &dmat_h);

std::tuple<Signal2D, Signal2D, Signal2D, Signal2D> dwt2(
    Signal2D const &x, Signal2DCompressed const &dmat_w,
    Signal2DCompressed const &dmat_h);

Signal2D idwt2(const Signal2D &ll, const Signal2D &lh, const Signal2D &hl,
               const Signal2D &hh, const Signal2DCompressed &dmat_w,
               const Signal2DCompressed &dmat_h);

/**
 * Construct the scaling filter associated with the Daubechies wavelet
 * @param wnum Daubechies wavelet vanishing moments, positive integer in the
 * closed interval [1, 10]
 * @return Scaling filter values
 */
Signal1D dbwavf(const int wnum);

/**
 * Computes the four lowpass and highpass, decomposition and reconstruction
 * filters associated with the scaling filter W corresponding to a wavelet
 * @param W_in Scaling filter corresponding to a wavelet
 * @return Decomposition: lowpass, highpass; Reconstruction: lowpass, highpass
 */
std::tuple<Signal1D, Signal1D, Signal1D, Signal1D> Orthfilt(
    Signal1D const &W_in);


/**
 * Wavelet decomposition using Daubechies matrix
 * @param signal
 * @param dmat convolutional matrix / 2 row with Lo_D, Hi_D filters
 * @return
 */
std::tuple<blaze::DynamicVector<DataType>, blaze::DynamicVector<DataType>> dwt(
    const blaze::DynamicVector<DataType>& signal,
    const blaze::CompressedMatrix<DataType>& dmat);

/**
 * Wavelet composition using Daubechies matrix
 * @param low_subband
 * @param high_subband
 * @param dmat convolutional matrix / 2 row with Lo_R, Hi_R filters
 * @return
 */
blaze::DynamicVector<DataType> idwt(
    const blaze::DynamicVector<DataType>& low_subband,
    const blaze::DynamicVector<DataType>& high_subband,
    const blaze::CompressedMatrix<DataType>& dmat);
}  // namespace drift::wavelet
#endif  // WAVELET_BUFFER_WAVELET_H_
