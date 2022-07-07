// Copyright 2021 PANDA GmbH

#ifndef WAVELET_BUFFER_PADDING_H_
#define WAVELET_BUFFER_PADDING_H_

#include <blaze/Blaze.h>

#include "wavelet_buffer/primitives.h"

namespace drift {
/** @enum PaddingLocation
 * \brief Padding signal location
 * For non zero reminder of division of padding size by 2, right and bottom
 * padding size larger by this reminder
 */
enum class PaddingLocation { kRight, kBoth };

/**
 * Base class for padding algorithms
 */
class PaddingAlgorithm {
 public:
  /** Constructor
   * @param rows padded rows number
   * @param columns padded columns number
   */
  PaddingAlgorithm(size_t rows, size_t columns,
                   PaddingLocation location = PaddingLocation::kBoth);

  /**
   * Pad signal
   * @param source input signal
   * @return padded signal
   */
  virtual blaze::DynamicMatrix<DataType> Extend(
      const blaze::DynamicMatrix<DataType>& source) const = 0;

  /**
   * Crop signal
   * @param source input signal
   * @return cropped signal
   */
  blaze::DynamicMatrix<DataType> Crop(
      const blaze::DynamicMatrix<DataType>& padded) const;

 protected:
  size_t rows_, columns_;
  PaddingLocation location_;
};

/**
 * Pad signal with zero values
 */
class ZeroPaddingAlgorithm : public PaddingAlgorithm {
 public:
  using PaddingAlgorithm::PaddingAlgorithm;
  blaze::DynamicMatrix<DataType> Extend(
      const blaze::DynamicMatrix<DataType>& source) const override;
};

/**
 * \brief Pad signal with zero derivative values
 * Means that signal marginal values are repeated
 */
class ZeroDerivativePaddingAlgorithm : public PaddingAlgorithm {
 public:
  using PaddingAlgorithm::PaddingAlgorithm;
  blaze::DynamicMatrix<DataType> Extend(
      const blaze::DynamicMatrix<DataType>& source) const override;
};

}  // namespace drift

#endif  // WAVELET_BUFFER_PADDING_H_
