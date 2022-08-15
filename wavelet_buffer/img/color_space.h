// Copyright 2020-2021 PANDA GmbH

#ifndef WAVELET_BUFFER_IMG_COLOR_SPACE_H_
#define WAVELET_BUFFER_IMG_COLOR_SPACE_H_

#include <blaze/Blaze.h>

#include "wavelet_buffer/primitives.h"

namespace drift::img {

enum ColorSpace { kRGB = 1, kHSL = 2, kGray = 3 };

/**
 * Convert pixel from RGB to HSL color space (all values are normalized)
 * @param red
 * @param green
 * @param blue
 * @param hue
 * @param saturation
 * @param luminance
 */
void RGBToHSL(DataType red, DataType green, DataType blue, DataType* hue,
              DataType* saturation, DataType* luminance);

/**
 * Convert pixel from HSL to RGB color space (all values are normalized)
 * @param hue
 * @param saturation
 * @param luminance
 * @param red
 * @param green
 * @param blue
 */
void HSLToRGB(DataType hue, DataType saturation, DataType luminance,
              DataType* red, DataType* green, DataType* blue);

/**
 * Convert RGB blaze matrix to HSL matrix
 * @param RGB Image matrix
 * @param start_channel
 */
void ConvertRgbToHsl(
    blaze::DynamicVector<blaze::DynamicMatrix<DataType>>* image,
    size_t start_channel = 0);

/**
 * Convert HSL blaze matrix to RGB matrix
 * @param RGB Image matrix
 * @param start_channel
 */
void ConvertHslToRgb(
    blaze::DynamicVector<blaze::DynamicMatrix<DataType>>* image,
    size_t start_channel = 0);

}  // namespace drift::img

#endif  // WAVELET_BUFFER_IMG_COLOR_SPACE_H_
