// Copyright 2020-2022 PANDA GmbH

#ifndef WAVELET_BUFFER_WAVELET_BUFFER_SERIALIZER_H_
#define WAVELET_BUFFER_WAVELET_BUFFER_SERIALIZER_H_

#include <memory>
#include <string>

#include "wavelet_buffer/wavelet_buffer.h"

namespace drift {
class IWaveletBufferSerializer {
 public:
  virtual ~IWaveletBufferSerializer() = default;
  /**
   * Parses subbands from a blob of data and creates a new buffer
   * @param blob the blob of subbands
   * @return nullptr if it failed to parse the buffer
   */
  [[nodiscard]] virtual std::unique_ptr<WaveletBuffer> Parse(
      const std::string& blob) = 0;
  /**
   * Serialize the buffer into the blob for saving in a file or sending via
   * network
   * @param blob the blob to serialize
   * @param sf_compression - 0 - switch off, 16 - max compression(bfloat).
   * @return return true if it has no error
   */
  [[nodiscard]] virtual bool Serialize(const WaveletBuffer& buffer,
                                       std::string* blob,
                                       uint8_t sf_compression = 0) = 0;
};

class WaveletBufferSerializerLegacy : public IWaveletBufferSerializer {
 public:
  /**
   * Parses subbands from a blob of data and creates a new buffer
   * @param blob the blob of subbands
   * @return nullptr if it failed to parse the buffer
   */
  [[nodiscard]] std::unique_ptr<WaveletBuffer> Parse(
      const std::string& blob) override;
  /**
   * Serialize the buffer into the blob for saving in a file or sending via
   * network
   * @param blob the blob to serialize
   * @param sf_compression - 0 - switch off, 16 - max compression(bfloat).
   * @return return true if it has no error
   */
  [[nodiscard]] bool Serialize(const WaveletBuffer& buffer, std::string* blob,
                               uint8_t sf_compression = 0) override;
};

class WaveletBufferSerializer : public IWaveletBufferSerializer {
 public:
  /**
   * Parses subbands from a blob of data and creates a new buffer
   * @param blob the blob of subbands
   * @return nullptr if it failed to parse the buffer
   */
  [[nodiscard]] std::unique_ptr<WaveletBuffer> Parse(
      const std::string& blob) override;
  /**
   * Serialize the buffer into the blob for saving in a file or sending via
   * network
   * @param blob the blob to serialize
   * @param sf_compression - 0 - switch off, 16 - max compression(bfloat).
   * @return return true if it has no error
   */
  [[nodiscard]] bool Serialize(const WaveletBuffer& buffer, std::string* blob,
                               uint8_t sf_compression = 0) override;
};
}  // namespace drift

#endif  // WAVELET_BUFFER_WAVELET_BUFFER_SERIALIZER_H_
