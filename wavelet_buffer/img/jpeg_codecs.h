// Copyright 2020-2022 PANDA GmbH

#ifndef WAVELET_BUFFER_IMG_JPEG_CODECS_H_
#define WAVELET_BUFFER_IMG_JPEG_CODECS_H_

#include <blaze/Blaze.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/gil/extension/io/jpeg.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/typedefs.hpp>

#include "wavelet_buffer/img/color_space.h"
#include "wavelet_buffer/img/image_codec.h"

namespace drift::img {
/**
 * Class to read and write JPEGs by using RGB color scheme
 */
class RgbJpegCodec : public IImageCodec {
 public:
  RgbJpegCodec();
  explicit RgbJpegCodec(float quality);

  [[nodiscard]] bool Decode(const std::string& blob, SignalN2D* image,
                            size_t start_channel = 0) const override;

  [[nodiscard]] bool Encode(const SignalN2D& image, std::string* blob,
                            size_t start_channel = 0) const override;

  [[nodiscard]] size_t channel_number() const override { return 3; }

 private:
  float quality_;
};

/**
 * Class to read and write JPEGs by using HSL color scheme
 */
class HslJpegCodec : public IImageCodec {
 public:
  HslJpegCodec();
  explicit HslJpegCodec(float quality);

  [[nodiscard]] bool Decode(const std::string& blob, SignalN2D* image,
                            size_t start_channel = 0) const override;

  [[nodiscard]] bool Encode(const SignalN2D& image, std::string* blob,
                            size_t start_channel = 0) const override;

  [[nodiscard]] size_t channel_number() const override { return 3; }

 private:
  float quality_;
};

/**
 * Class to read and write JPEGs by using gray color scheme
 */
class GrayJpegCodec : public IImageCodec {
 public:
  GrayJpegCodec();
  explicit GrayJpegCodec(float quality);

  [[nodiscard]] bool Decode(const std::string& blob, SignalN2D* image,
                            size_t start_channel = 0) const override;

  [[nodiscard]] bool Encode(const SignalN2D& image, std::string* blob,
                            size_t start_channel = 0) const override;

  [[nodiscard]] size_t channel_number() const override { return 1; }

 private:
  float quality_;
};

}  // namespace drift::img

#endif  // WAVELET_BUFFER_IMG_JPEG_CODECS_H_
