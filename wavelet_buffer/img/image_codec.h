// Copyright 2020-2022 PANDA GmbH

#ifndef WAVELET_BUFFER_IMG_IMAGE_CODEC_H_
#define WAVELET_BUFFER_IMG_IMAGE_CODEC_H_

#include <blaze/Blaze.h>

#include <string>

#include "wavelet_buffer/primitives.h"

namespace drift::img {
/**
 * Abstract interface for image codecs
 */
class IImageCodec {
 public:
  virtual ~IImageCodec() = default;

  /**
   * Decodes a picture from a string to blaze matrices
   * @param blob the string with the picture in some image format
   * @param image a pointer to decoded blaze image
   * @param start_channel
   * @return  false if there is an error
   */
  virtual bool Decode(const std::string& blob, SignalN2D* image,
                      size_t start_channel = 0) const = 0;

  /**
   * Encodes a blaze image to string in some image format
   * @param image the blaze image to encoded
   * @param blob an encoded string
   * @param start_channel
   * @return  false if there is an error
   */
  virtual bool Encode(const SignalN2D& image, std::string* blob,
                      size_t start_channel = 0) const = 0;

  virtual size_t channel_number() const = 0;

  [[nodiscard]] virtual bool checkChannelsShape(const SignalN2D& image,
                                                size_t start_channel) const = 0;
};
}  // namespace drift::img
#endif  // WAVELET_BUFFER_IMG_IMAGE_CODEC_H_
