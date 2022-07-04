// Copyright 2021 PANDA GmbH
#ifndef WAVELET_BUFFER_WAVELET_UTILS_H_
#define WAVELET_BUFFER_WAVELET_UTILS_H_

#include <blaze/Blaze.h>

#include <tuple>
#include <vector>

#include "wavelet_buffer/denoise_algorithms.h"
#include "wavelet_buffer/padding.h"
#include "wavelet_buffer/primitives.h"
#include "wavelet_buffer/wavelet_parameters.h"

namespace drift {

class WaveletBuffer;

using Subband = blaze::DynamicMatrix<DataType>;
using WaveletDecomposition = blaze::DynamicVector<Subband>;
using NWaveletDecomposition = blaze::DynamicVector<WaveletDecomposition>;
using Padding = ZeroDerivativePaddingAlgorithm;

namespace internal {

/**
 * @brief Calculate the maximum possible decomposition steps depending on the
 * wavelet type and signal size
 * @return maximum decomposition steps
 */
int CalculateMaxDecompositionSteps(WaveletTypes wavelet_type,
                                   const std::vector<size_t>& signal_shape);

/**
 * Partial decompose
 * @param parameters wavelet parameters
 * @param decomposition the initial decomposition to decompose
 * @param steps
 * @param denoiser
 */
void DecomposeImpl(WaveletParameters parameters,
                   NWaveletDecomposition* decomposition, int steps,
                   const DenoiseAlgorithm<DataType>& denoiser);

/**
 * Decompose signal
 * @param parameters
 * @param decomposition
 * @param data
 * @param denoiser
 * @param start_signal
 * @param signal_count
 * @return
 */
bool DecomposeImpl(const WaveletParameters& parameters,
                   NWaveletDecomposition* decomposition, const SignalN2D& data,
                   const DenoiseAlgorithm<DataType>& denoiser,
                   size_t start_signal, size_t signal_count);

/**
 * Partial compose
 * @param params wavelet parameters of the decomposition
 * @param decomposition the wavelet subband
 * @param start_signal the first signal in the vector
 * @param count the number of signals to decompose
 * @return new decomposition
 */
NWaveletDecomposition ComposeImpl(const WaveletParameters& params,
                                  const NWaveletDecomposition& decomposition,
                                  size_t steps, size_t start_channel,
                                  size_t count);

/**
 * Compose signals from decomposition
 * @param params wavelet parameters of the decomposition
 * @param data a pointer to the composed signal
 * @param decomposition the wavelet subband
 * @param start_signal the first signal in the vector
 * @param count the number of signals to decompose
 * @return false if there is an error
 */
bool ComposeImpl(const WaveletParameters& params, SignalN2D* data,
                 const NWaveletDecomposition& decomposition, size_t steps,
                 size_t start_signal, size_t count);


/**
 * Remove padding depending on signal shape and dimension
 * @tparam Container type of Matrix
 * @param data
 * @param padded_size
 * @return
 */
template <typename Container>
void CropPadding(Container* data, const SignalShape& signal_shape) {
  size_t rows, columns;
  if (signal_shape.size() > 1) {
    rows = signal_shape[1];
    columns = signal_shape[0];
  } else {
    rows = signal_shape[0];
    columns = 1;
  }

  Padding padding(rows, columns);
  *data = padding.Crop(*data);
}

/**
 * Add padding depending on signal shape and dimension
 * @tparam Container type of Matrix
 * @param data
 * @param padded_size
 * @return
 */
template <typename Container>
static Container AddPadding(const Container& data,
                            const SignalShape& padded_size) {
  size_t rows, columns;
  if (padded_size.size() > 1) {
    rows = padded_size[1];
    columns = padded_size[0];
  } else {
    rows = padded_size[0];
    columns = 1;
  }

  Padding padding(rows, columns);
  return padding.Extend(data);
}

/**
 * Calculate padding for maximum step number
 * @param wavelet_type the type that has different maximum
 * @param signal_shape
 * @param decomposition_steps
 * @return
 */
SignalShape CalcPaddedSize(WaveletTypes wavelet_type,
                           const SignalShape& signal_shape,
                           const int decomposition_steps);

/**
 * Number of subbands that will be added on each wavelet decomposition
 * @param parameters
 * @return
 */
int SubbandsPerWaveletTransform(const WaveletParameters& parameters);
}  // namespace internal

/**
 * Estimate number of subbands for specific parameters
 * @param parameters
 * @return
 */
size_t DecompositionSize(const WaveletParameters& parameters);

/**
 * Compare the buffer to another one.
 * @note The buffers must have the same number of decomposition steps
 * otherwise the function returns NaN
 * @return distance value 0 both buffers the same, 1 both buffers absolutely
 * different
 */
DataType Distance(const WaveletBuffer& lhs, const WaveletBuffer& rhs);

/**
 * Calculate energy distribution by subbands of WaveletBuffer
 *
 * @param buffer source of subbunds
 * @return vector of vector of the energies , layout is the same with
 * WaveletBuffer
 */
blaze::DynamicVector<blaze::DynamicVector<DataType>> EnergyDistribution(
    const WaveletBuffer& buffer);

}  // namespace drift

#endif  // WAVELET_BUFFER_WAVELET_UTILS_H_
