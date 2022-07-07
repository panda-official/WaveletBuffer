// Copyright 2020-2021 PANDA GmbH

#ifndef WAVELET_BUFFER_WAVELET_BUFFER_H_
#define WAVELET_BUFFER_WAVELET_BUFFER_H_

#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "wavelet_buffer/denoise_algorithms.h"
#include "wavelet_buffer/primitives.h"
#include "wavelet_buffer/wavelet_parameters.h"
#include "wavelet_buffer/wavelet_utils.h"

namespace drift {

constexpr uint8_t kSerializationVersion =
    2;  // Increase if we brake compatibility

class WaveletBufferView;
/**
 * Universal buffer for the wavelet decomposition
 */
class WaveletBuffer {
 public:
  /**
   * Initialize buffer
   * @param parameters the parameters of the wavelet decomposition
   */
  explicit WaveletBuffer(const WaveletParameters& parameters);

  /**
   * Initialize buffer with decompositions
   * @param parameters the parameters of the wavelet decomposition
   * @param decompositions
   */
  WaveletBuffer(const WaveletParameters& parameters,
                const NWaveletDecomposition& decompositions);

  /**
   *
   * @param buffer
   */
  WaveletBuffer(WaveletBuffer&& buffer) noexcept;

  /**
   *
   * @param buffer
   */
  WaveletBuffer(const WaveletBuffer& buffer);

  /**
   * Destructor, required by std::unique_ptr and PImpl
   */
  ~WaveletBuffer();

  /**** Wavelet transformation ********/

  /**
   * Decomposes the signal into the subbands and saves it internally
   * @param data the signal
   * @param denoiser algorithm to clean the small values in Hi-freq
   * subbands
   * @return true if it has no errors
   */
  bool Decompose(const SignalN2D& data,
                 const DenoiseAlgorithm<DataType>& denoiser);

  /**
   * Decomposes the signal into the subbands and saves it internally
   * @param data the signal
   * @param denoiser algorithm to clean the small values in Hi-freq
   * subbands
   * @return true if it has no errors
   */
  bool Decompose(const Signal1D& data,
                 const DenoiseAlgorithm<DataType>& denoiser);

  /**
   * Composes the intrnal subbands into a signal
   * @param data the signal
   * @param scale_factor wavelet scale factor 0 - all the steps
   * of the decomposition recomposed and the output has size of the original
   * signal. If factor N - all the step - N are recomposed and output has size
   * 2^N smaller
   * @return true if it has no errors
   */
  bool Compose(SignalN2D* data, int scale_factor = 0) const;

  /**
   * Composes the intrnal subbands into a signal
   * @param data the signal
   * @param scale_factor wavelet scale factor 0 - all the steps
   * of the decomposition recomposed and the output has size of the original
   * signal. If factor N - all the step - N are recomposed and output has size
   * 2^N smaller
   * @return true if it has no errors
   */
  bool Compose(Signal1D* data, int scale_factor = 0) const;

  /******** Serializers **************/

  /**
   * Parses subbands from a blob of data and creates a new buffer
   * @param blob the blob of subbands
   * @return nullptr if it failed to parse the buffer
   */
  [[nodiscard]] static std::unique_ptr<WaveletBuffer> Parse(
      const std::string& blob);

  /**
   * Serialize the buffer into the blob for saving in a file or sending via
   * network
   * @param blob the blob to serialize
   * @param sf_compression - 0 - switch off, 16 - max compression(bfloat).
   * @return return true if it has no error
   */
  [[nodiscard]] bool Serialize(std::string* blob,
                               uint8_t sf_compression = 0) const;

  /******** Accessors ***************/

  /**
   * Access to the decomposition by channels
   * NOTE: the last element is always an abstraction
   * @param index the index of the channel
   * @return the decomposition as a list of the subbands
   */
  [[nodiscard]] WaveletDecomposition& operator[](int index);

  /**
   * Const access to the decomposition by channels
   * @param index the index of the channel
   * @return the decomposition as a list of the subbands
   */
  [[nodiscard]] const WaveletDecomposition& operator[](int index) const;

  /**
   * Makes a view for (index, index+count) signals
   * @param index
   * @param count
   * @return
   */
  WaveletBufferView operator()(int index, int count);

  /**
   * Makes a const view for (index, index+count) signals
   * @param index
   * @param count
   * @return
   */
  WaveletBufferView operator()(int index, int count) const;

  /**
   * Parameters of the decomposition
   */
  [[nodiscard]] const WaveletParameters& parameters() const;

  [[nodiscard]] NWaveletDecomposition& decompositions();
  [[nodiscard]] const NWaveletDecomposition& decompositions() const;

  bool operator==(const WaveletBuffer& rhs) const;

  bool operator!=(const WaveletBuffer& rhs) const;

  WaveletBuffer& operator=(const WaveletBuffer& rhs);

  WaveletBuffer& operator=(WaveletBuffer&& rhs) noexcept;

  /******** Helpers ****************/
  /**
   * Gets min and max factors of values in subband
   * example:
   *    if your signal has values in interval 0..5, so the pair (-2, 2) means
   *    that you a subband has values in interval -10..10
   * @param index the index of the subband
   * @return a pair of min and max factors
   */
  [[nodiscard]] std::pair<DataType, DataType> GetValueRange(size_t index) const;

  /**
   * Check that buffer have decomposed data
   * @return
   */
  bool IsEmpty() const;

  friend std::ostream& operator<<(std::ostream&, const WaveletBuffer& wb);

 private:
  WaveletBuffer() = default;

  class Impl;

  std::unique_ptr<Impl> impl_;
};

}  // namespace drift

#endif  // WAVELET_BUFFER_WAVELET_BUFFER_H_
