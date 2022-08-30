// Copyright 2020-2022 PANDA GmbH

#include "wavelet_buffer/img/jpeg_codecs.h"

#include <iostream>
#include <string>

#include <boost/gil/extension/io/jpeg.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/typedefs.hpp>

#define cimg_plugin "plugins/jpeg_buffer.h"
#include "CImg.h"

#include "wavelet_buffer/img/color_space.h"

namespace drift::img {

namespace bg = boost::gil;
using namespace cimg_library;

void CheckRangeQuality(DataType quality) {
  if ((quality < 0) || (quality > 1.f)) {
    throw std::runtime_error("write_quality out of range 0..1");
  }
}

template <typename T>
std::uint8_t ToPixel(T x) {
  x = std::max<T>(0, std::min<T>(1, x));
  return x * 255;
}

RgbJpegCodec::RgbJpegCodec() : RgbJpegCodec(1.f) {}

RgbJpegCodec::RgbJpegCodec(DataType quality) : quality_(quality) {
  CheckRangeQuality(quality);
}

bool RgbJpegCodec::Decode(const std::string& blob, SignalN2D* image,
                          size_t start_channel) const {
  if (blob.empty()) {
    std::cerr << "Failed to decode image: buffer is empty" << std::endl;
    return false;
  }

  CImg<unsigned char> img;
  try {
    img.load_jpeg_buffer(reinterpret_cast<const JOCTET*>(blob.data()), blob.size());
  } catch (std::exception& e) {
    std::cerr << "Failed to decode image: " << e.what() << std::endl;
    return false;
  }

  const auto rows = img.height();
  const auto columns = img.width();

  auto& im = *image;
  if (im.size() < start_channel + 3) {
    im.resize(start_channel + 3);
  }

  for (size_t ch = start_channel; ch < start_channel + 3; ++ch) {
    im[ch] = Signal2D(rows, columns);

    for (size_t i = 0; i < rows; ++i) {
      for (size_t j = 0; j < columns; ++j) {
        im[ch](i, j) = img(j, i, ch - start_channel) / 255.;
      }
    }
  }

  return true;
}

bool RgbJpegCodec::Encode(const SignalN2D& image, std::string* blob,
                          size_t start_channel) const {
  if (!checkChannelsShape(image, start_channel)) {
    return false;
  }

  const size_t rows = image[start_channel].rows();
  const size_t columns = image[start_channel].columns();


  CImg<unsigned char> img(columns, rows, 1, 3);
  for (size_t c = start_channel; c < start_channel + 3; c++) {
    for (size_t i = 0; i < rows; i++) {
      for (size_t j = 0; j < columns; j++) {
        img(j, i, c  - start_channel) = ToPixel(image[c](i, j));
      }
    }
  }
  unsigned int buffer_size = image.size() * rows * columns + 1000;
  blob->resize(buffer_size);
  img.save_jpeg_buffer(reinterpret_cast<JOCTET*>(blob->data()), buffer_size, static_cast<int>(quality_ * 100));
  blob->resize(buffer_size);

  return true;
}

bool RgbJpegCodec::checkChannelsShape(const SignalN2D& image,
                                      size_t start_channel) const {
  if (image.size() < channel_number() + start_channel) {
    std::cerr << "Failed to encode: image must at least "
              << channel_number() + start_channel << "channels (currently "
              << image.size() << ")" << std::endl;
    return false;
  }

  const size_t rows0 = image[start_channel].rows();
  const size_t columns0 = image[start_channel].columns();
  for (size_t i = start_channel + 1; i < start_channel + 3; ++i) {
    if ((image[i].rows() != rows0) || (image[i].columns() != columns0)) {
      std::cerr << "Failed to encode: channel has different size" << std::endl;
      return false;
    }
  }

  if ((rows0 == 0) || (columns0 == 0)) {
    std::cerr << "Failed to encode: 0x0 image" << std::endl;
    return false;
  }

  return true;
}

HslJpegCodec::HslJpegCodec() : HslJpegCodec(1.f) {}

HslJpegCodec::HslJpegCodec(DataType quality) : quality_(quality) {
  CheckRangeQuality(quality);
}
bool HslJpegCodec::Decode(const std::string& blob, SignalN2D* image,
                          size_t start_channel) const {
  RgbJpegCodec rgb_codec(quality_);

  if (rgb_codec.Decode(blob, image, start_channel)) {
    ConvertRgbToHsl(image, start_channel);
  } else {
    return false;
  }

  return true;
}

bool HslJpegCodec::Encode(const SignalN2D& image, std::string* blob,
                          size_t start_channel) const {
  if (!checkChannelsShape(image, start_channel)) {
    return false;
  }

  RgbJpegCodec rgb_codec(quality_);
  auto cpy = image;  // TODO(Aleksey Timin): We should find a way to avoid copy
  ConvertHslToRgb(&cpy, start_channel);
  return rgb_codec.Encode(cpy, blob, start_channel);
}

bool HslJpegCodec::checkChannelsShape(const SignalN2D& image,
                                      size_t start_channel) const {
  if (image.size() < channel_number() + start_channel) {
    std::cerr << "Failed to encode: image must at least "
              << channel_number() + start_channel << "channels (currently "
              << image.size() << ")" << std::endl;
    return false;
  }

  const size_t rows0 = image[start_channel].rows();
  const size_t columns0 = image[start_channel].columns();
  for (size_t i = start_channel + 1; i < start_channel + 3; ++i) {
    if ((image[i].rows() != rows0) || (image[i].columns() != columns0)) {
      std::cerr << "Failed to encode: channel has different size" << std::endl;
      return false;
    }
  }

  if ((rows0 == 0) || (columns0 == 0)) {
    std::cerr << "Failed to encode: 0x0 image" << std::endl;
    return false;
  }

  return true;
}

GrayJpegCodec::GrayJpegCodec() : quality_(1.f) {}

GrayJpegCodec::GrayJpegCodec(DataType quality) : quality_(quality) {
  CheckRangeQuality(quality);
}

bool GrayJpegCodec::Decode(const std::string& blob, SignalN2D* img,
                           size_t start_channel) const {
  bg::image_read_settings<bg::jpeg_tag> read_settings;
  bg::gray8_image_t bg_image;

  static_assert(
      bg::num_channels<decltype(bg_image)::view_t::value_type>::value == 1,
      "Image must have 1 channels");

  try {
    std::istringstream iss(blob);
    bg::read_image(iss, bg_image, read_settings);

    const auto& view = bg::view(bg_image);

    auto& im = *img;
    if (im.size() < 1 + start_channel) {
      im.resize(start_channel + 1);
    }
    im[start_channel] = Signal2D(view.height(), view.width());

    for (size_t y = 0; y < view.height(); ++y) {
      for (size_t x = 0; x < view.width(); ++x) {
        const auto& p = view(x, y);
        im[start_channel](y, x) = static_cast<DataType>(p) / 255;
      }
    }
  } catch (std::exception const& e) {
    std::cerr << "Decode exception: " << e.what() << std::endl;
    return false;
  }

  return true;
}

bool GrayJpegCodec::Encode(const SignalN2D& image, std::string* blob,
                           size_t start_channel) const {
  if (!checkChannelsShape(image, start_channel)) {
    return false;
  }

  try {
    bg::gray8_image_t gil_img(image[start_channel].columns(),
                              image[start_channel].rows());
    auto& view = bg::view(gil_img);

    for (size_t y = 0; y < view.height(); ++y) {
      for (size_t x = 0; x < view.width(); ++x) {
        view(x, y) = ToPixel(image[start_channel](y, x));
      }
    }
    std::stringstream write_stream;
    bg::write_view(write_stream, view,
                   bg::image_write_info<bg::jpeg_tag>(quality_ * 100));
    *blob = std::string((std::istreambuf_iterator<char>(write_stream)),
                        std::istreambuf_iterator<char>());
  } catch (std::exception const& e) {
    std::cerr << "Encode exception: " << e.what() << std::endl;
    return false;
  }

  return true;
}

[[nodiscard]] bool GrayJpegCodec::checkChannelsShape(
    const SignalN2D& image, size_t start_channel) const {
  if (image.size() < start_channel + 1) {
    std::cerr << "Image must have at least 1 channel";
    return false;
  }

  if (image[start_channel].columns() * image[start_channel].rows() == 0) {
    std::cerr << "Image mustn't be empty";
    return false;
  }

  return true;
}

}  // namespace drift::img
