// Copyright 2021 PANDA GmbH

#include "wavelet_buffer/padding.h"

namespace drift {

PaddingAlgorithm::PaddingAlgorithm(size_t rows, size_t columns,
                                   PaddingLocation location)
    : rows_(rows), columns_(columns), location_(location) {}

blaze::DynamicMatrix<DataType> PaddingAlgorithm::Crop(
    const blaze::DynamicMatrix<DataType> &source) const {
  assert(columns_ <= source.columns() && rows_ <= source.rows() &&
         "Crop can only be done if the source is bigger then the new "
         "size");

  if (rows_ * columns_ == 0) {
    // empty matrix is valid input for kNone wavelet type, bypass
    return source;
  }

  size_t row_0 = 0;
  size_t column_0 = 0;

  if (location_ == PaddingLocation::kBoth) {
    row_0 = (source.rows() - rows_) / 2;
    column_0 = (source.columns() - columns_) / 2;
  }

  blaze::DynamicMatrix<DataType> result =
      submatrix(source, row_0, column_0, rows_, columns_);

  return result;
}

blaze::DynamicMatrix<DataType> ZeroPaddingAlgorithm::Extend(
    const blaze::DynamicMatrix<DataType> &source) const {
  assert(columns_ >= source.columns() && rows_ >= source.rows() &&
         "Padding can only be done if the source is smaller then the new "
         "size");

  if (rows_ * columns_ == 0) {
    // empty matrix is valid input fpr kNone wavelet type, bypass
    return source;
  }

  blaze::DynamicMatrix<DataType> result =
      blaze::zero<DataType>(rows_, columns_);

  size_t row_0 = 0;
  size_t column_0 = 0;

  if (location_ == PaddingLocation::kBoth) {
    row_0 = (rows_ - source.rows()) / 2;
    column_0 = (columns_ - source.columns()) / 2;
  }

  submatrix(result, row_0, column_0, source.rows(), source.columns()) = source;

  return result;
}

blaze::DynamicMatrix<DataType> ZeroDerivativePaddingAlgorithm::Extend(
    const blaze::DynamicMatrix<DataType> &source) const {
  assert(columns_ >= source.columns() && rows_ >= source.rows() &&
         "Padding can only be done if the source is smaller then the new "
         "size");

  if (rows_ * columns_ == 0) {
    // empty matrix is valid input fpr kNone wavelet type, bypass
    return source;
  }

  /* Calculate deltas */
  const size_t dc = columns_ - source.columns();
  const size_t dr = rows_ - source.rows();

  /* Calculate padding sizes */
  size_t left_padding, right_padding, top_padding, bottom_padding;
  if (location_ == PaddingLocation::kRight) {
    left_padding = 0;
    top_padding = 0;
    right_padding = dc;
    bottom_padding = dr;
  } else if (location_ == PaddingLocation::kBoth) {
    left_padding = dc / 2;
    top_padding = dr / 2;
    right_padding = dc / 2 + dc % 2;
    bottom_padding = dr / 2 + dr % 2;
  } else {
    return {};
  }

  /* Fill original part */
  blaze::DynamicMatrix<DataType> result(rows_, columns_);
  submatrix(result, top_padding, left_padding, source.rows(),
            source.columns()) = source;

  /* Get four edges */
  const auto left_edge = blaze::submatrix(source, 0, 0, source.rows(), 1);
  const auto right_edge =
      blaze::submatrix(source, 0, source.columns() - 1, source.rows(), 1);
  const auto bottom_edge =
      blaze::submatrix(source, source.rows() - 1, 0, 1, source.columns());
  const auto top_edge = blaze::submatrix(source, 0, 0, 1, source.columns());

  /* Get four corner values */
  const auto bottom_right_corner =
      source(source.rows() - 1, source.columns() - 1);
  const auto top_left_corner = source(0, 0);
  const auto top_righ_corner = source(0, source.columns() - 1);
  const auto bottom_left_corner = source(source.rows() - 1, 0);

  /* Extend: */

  /* Right part */
  if (right_padding > 0) {
    submatrix(result, top_padding, left_padding + source.columns(),
              source.rows(), right_padding) =
        blaze::repeat(right_edge, 1, right_padding);
  }
  /* Extend bottom part */
  if (bottom_padding > 0) {
    submatrix(result, top_padding + source.rows(), left_padding, bottom_padding,
              source.columns()) = blaze::repeat(bottom_edge, bottom_padding, 1);
  }
  /* Extend left part */
  if (left_padding > 0) {
    submatrix(result, top_padding, 0, source.rows(), left_padding) =
        blaze::repeat(left_edge, 1, left_padding);
  }
  /* Extend top part */
  if (top_padding > 0) {
    submatrix(result, 0, left_padding, top_padding, source.columns()) =
        blaze::repeat(top_edge, top_padding, 1);
  }

  /* Right bottom corner part */
  if ((bottom_padding > 0) && (right_padding > 0)) {
    submatrix(result, top_padding + source.rows(),
              left_padding + source.columns(), bottom_padding, right_padding) =
        bottom_right_corner;
  }
  /* Left bottom corner part */
  if ((bottom_padding > 0) && (left_padding > 0)) {
    submatrix(result, top_padding + source.rows(), 0, bottom_padding,
              left_padding) = bottom_left_corner;
  }
  /* Left top corner part */
  if ((top_padding > 0) && (left_padding > 0)) {
    submatrix(result, 0, 0, top_padding, left_padding) = top_left_corner;
  }
  /* Right top corner part */
  if ((top_padding > 0) && (right_padding > 0)) {
    submatrix(result, 0, left_padding + source.columns(), top_padding,
              right_padding) = top_righ_corner;
  }

  return result;
}

}  // namespace drift
