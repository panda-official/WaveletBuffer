// Copyright 2020-2022 PANDA GmbH

#ifndef WAVELET_BUFFER_WAVELET_BUFFER_SERIALIZER_H_
#define WAVELET_BUFFER_WAVELET_BUFFER_SERIALIZER_H_

#include <memory>
#include <string>

#include "wavelet_buffer/wavelet_buffer.h"

namespace drift {
class WaveletBufferSerializer {
 public:
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
  [[nodiscard]] static bool Serialize(const WaveletBuffer& buffer,
                                      std::string* blob,
                                      uint8_t sf_compression = 0);
  /**
   * We need to allocate data for SfComprressor depending on subband size
   * @param signal_shape
   * @param sub_number number of suuband, if 0 it is the original signal
   * @return needed memory size of subband in bytes + 50% reserve
   */
  static size_t GetMemorySizeForSfCompressor(const SignalShape& signal_shape,
                                             int sub_number = 0);
};
}  // namespace drift

#endif  // WAVELET_BUFFER_WAVELET_BUFFER_SERIALIZER_H_
