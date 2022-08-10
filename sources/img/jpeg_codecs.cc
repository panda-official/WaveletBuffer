// Copyright 2020-2022 PANDA GmbH

#include "wavelet_buffer/img/jpeg_codecs.h"

#include <iostream>
#include <string>

#include <boost/gil/extension/io/jpeg.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/typedefs.hpp>

#include "wavelet_buffer/img/color_space.h"

namespace drift::img {

namespace bg = boost::gil;

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
  bg::rgb8_image_t bg_image;
  std::stringstream in_buffer(blob, std::ios_base::in | std::ios_base::binary);
  try {
    bg::read_image(in_buffer, bg_image, bg::jpeg_tag());
  } catch (std::exception& e) {
    std::cerr << "Failed to decode image: " << e.what() << std::endl;
    return false;
  }

  const auto rows = bg_image.height();
  const auto columns = bg_image.width();

  auto& im = *image;
  if (im.size() < start_channel + 3) {
    im.resize(start_channel + 3);
  }

  auto rgb_view = bg::view(bg_image);
  for (int ch = start_channel; ch < start_channel + 3; ++ch) {
    im[ch] = Signal2D(rows, columns);
    const auto& view_channel =
        bg::nth_channel_view(rgb_view, ch - start_channel);
    auto it_channel = view_channel.begin();

    for (int i = 0; i < rows; i++) {
      auto it_channel_end = it_channel + columns;
      std::transform(it_channel, it_channel_end, im[ch].begin(i),
                     [](auto x) { return static_cast<DataType>(x) / 255; });
      it_channel = it_channel_end;
    }
  }

  return true;
}

bool RgbJpegCodec::Encode(const SignalN2D& image, std::string* blob,
                          size_t start_channel) const {
  if (!checkChannelsShape(image, start_channel)) {
    return false;
  }

  const auto rows = image[start_channel].rows();
  const auto columns = image[start_channel].columns();

  bg::rgb8_image_t rgb_img(columns, rows);
  auto rgb_view = bg::view(rgb_img);
  for (int c = start_channel; c < start_channel + 3; c++) {
    const auto& view_channel =
        bg::nth_channel_view(rgb_view, c - start_channel);
    auto it = view_channel.begin();
    for (int i = 0; i < rows; i++) {
      it = std::transform(image[c].begin(i), image[c].end(i), it,
                          [](auto x) { return ToPixel(x); });
    }
  }

  std::stringstream out_buffer(std::ios_base::out | std::ios_base::binary);
  bg::write_view(out_buffer, rgb_view,
                 bg::image_write_info<bg::jpeg_tag>(quality_ * 100));

  *blob = out_buffer.str();
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
