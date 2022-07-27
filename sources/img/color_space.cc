// Copyright 2020-2022 PANDA GmbH

#include "wavelet_buffer/img/color_space.h"

#include <algorithm>

namespace drift::img {

void RGBToHSL(DataType red, DataType green, DataType blue, DataType* hue,
              DataType* saturation, DataType* luminance) {
  const auto min_color = std::min({red, green, blue});
  const auto max_color = std::max({red, green, blue});

  const DataType diff = max_color - min_color;
  const DataType sum = max_color + min_color;

  if (std::abs(diff) < 0.001f) {
    // rgb color is gray
    *hue = 0.f;
    *saturation = 0.f;
    *luminance = red;  // doesn't matter which rgb channel we use.
  } else {
    // Luminance calculation
    *luminance = sum / 2.f;

    // Saturation calculation
    if (*luminance < 0.5) {
      *saturation = diff / sum;
    } else {
      *saturation = diff / (2.f - sum);
    }

    // Hue calculation
    // max_color is red
    if (std::abs(max_color - red) < 0.0001f) {
      *hue = (green - blue) / diff;

      // max_color is green
    } else if (std::abs(max_color - green) < 0.0001f) {
      *hue = 2.f + (blue - red) / diff;

    } else {  // max_color is blue
      *hue = 4.f + (red - green) / diff;
    }

    *hue /= 6.f;

    if (*hue < 0.f) {
      *hue += 1.f;
    }
  }
}

void HSLToRGB(DataType hue, DataType saturation, DataType luminance,
              DataType* red, DataType* green, DataType* blue) {
  if (std::abs(saturation) < 0.0001) {
    // If saturation is 0, the color is a shade of gray
    *red = luminance;
    *green = luminance;
    *blue = luminance;
  } else {
    DataType temp1;
    // Set the temporary values
    if (luminance < 0.5) {
      temp1 = luminance * (1.f + saturation);
    } else {
      temp1 = (luminance + saturation) - (luminance * saturation);
    }

    const DataType temp2 = 2.f * luminance - temp1;

    DataType temp_red = hue + 1.f / 3.f;
    DataType temp_green = hue;
    DataType temp_blue = hue - 1.f / 3.f;
    if (temp_red > 1.f) {
      temp_red--;
    }
    if (temp_blue < 0.f) {
      temp_blue++;
    }

    // Red
    if (temp_red < 1.f / 6.f) {
      *red = temp2 + (temp1 - temp2) * 6.f * temp_red;
    } else if (temp_red < 0.5f) {
      *red = temp1;
    } else if (temp_red < 2.f / 3.f) {
      *red = temp2 + (temp1 - temp2) * ((2.f / 3.f) - temp_red) * 6.f;
    } else {
      *red = temp2;
    }

    // Green
    if (temp_green < 1.f / 6.f) {
      *green = temp2 + (temp1 - temp2) * 6.f * temp_green;
    } else if (temp_green < 0.5f) {
      *green = temp1;
    } else if (temp_green < 2.f / 3.f) {
      *green = temp2 + (temp1 - temp2) * ((2.f / 3.f) - temp_green) * 6.f;
    } else {
      *green = temp2;
    }

    // Blue
    if (temp_blue < 1.f / 6.f) {
      *blue = temp2 + (temp1 - temp2) * 6.f * temp_blue;
    } else if (temp_blue < 0.5f) {
      *blue = temp1;
    } else if (temp_blue < 2.f / 3.f) {
      *blue = temp2 + (temp1 - temp2) * ((2.f / 3.f) - temp_blue) * 6.f;
    } else {
      *blue = temp2;
    }
  }
}

void ConvertRgbToHsl(
    blaze::DynamicVector<blaze::DynamicMatrix<DataType>>* image,
    size_t start_channel) {
  assert(image && (image->size() >= 3 + start_channel) &&
         "must have at least 3 channels");

  auto& img = *image;
  DataType h, s, l;
  size_t width{img[start_channel].columns()}, height{img[start_channel].rows()};
  for (size_t y = 0; y < height; ++y) {
    for (size_t x = 0; x < width; ++x) {
      RGBToHSL(img[start_channel](y, x), img[start_channel + 1](y, x),
               img[start_channel + 2](y, x), &h, &s, &l);
      img[start_channel](y, x) = h;
      img[start_channel + 1](y, x) = s;
      img[start_channel + 2](y, x) = l;
    }
  }
}

void ConvertHslToRgb(
    blaze::DynamicVector<blaze::DynamicMatrix<DataType>>* image,
    size_t start_channel) {
  assert(image && (image->size() >= 3 + start_channel) &&
         "must have at least 3 channels");

  auto& img = *image;

  size_t width{img[start_channel].columns()}, height{img[start_channel].rows()};

  DataType h, s, l;
  for (size_t y = 0; y < height; ++y) {
    for (size_t x = 0; x < width; ++x) {
      HSLToRGB(img[start_channel](y, x), img[start_channel + 1](y, x),
               img[start_channel + 2](y, x), &h, &s, &l);
      img[start_channel](y, x) = h;
      img[start_channel + 1](y, x) = s;
      img[start_channel + 2](y, x) = l;
    }
  }
}

}  // namespace drift::img
