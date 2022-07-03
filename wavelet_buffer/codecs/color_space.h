// Copyright 2020-2021 PANDA GmbH

#ifndef WAVELET_BUFFER_CODECS_COLOR_SPACE_H_
#define WAVELET_BUFFER_CODECS_COLOR_SPACE_H_

#include <blaze/Blaze.h>

namespace drift::dsp::codecs {

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
void RGBToHSL(float red, float green, float blue, float* hue, float* saturation,
              float* luminance);

/**
 * Convert pixel from HSL to RGB color space (all values are normalized)
 * @param hue
 * @param saturation
 * @param luminance
 * @param red
 * @param green
 * @param blue
 */
void HSLToRGB(float hue, float saturation, float luminance, float* red,
              float* green, float* blue);

/**
 * Convert RGB blaze matrix to HSL matrix
 * @param RGB Image matrix
 * @param start_channel
 */
void ConvertRgbToHsl(blaze::DynamicVector<blaze::DynamicMatrix<float>>* image,
                     size_t start_channel = 0);

/**
 * Convert HSL blaze matrix to RGB matrix
 * @param RGB Image matrix
 * @param start_channel
 */
void ConvertHslToRgb(blaze::DynamicVector<blaze::DynamicMatrix<float>>* image,
                     size_t start_channel = 0);

}  // namespace drift::dsp::codecs

#endif  // WAVELET_BUFFER_CODECS_COLOR_SPACE_H_
