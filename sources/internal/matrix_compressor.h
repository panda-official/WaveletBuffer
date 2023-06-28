// Copyright 2022-2023 PANDA GmbH

#pragma once

#include <blaze/Blaze.h>

#include <tuple>
#include <vector>

namespace drift::wavelet::internal {

/* Compressed matrix data */
struct ArchivedMatrix {
  bool is_valid{false}; /**< state */
  size_t nonzero{0};    /**< number of non-zero elements */
  size_t rows_number{0};
  size_t cols_number{0};        /**< matrix columns */
  std::vector<uint8_t> indexes; /**< encoded  indexes */
  std::vector<uint8_t> values;  /**< encoded values */
};

class BlazeCompressor {
 public:
  BlazeCompressor() = default;

  /**
   * Compress a blaze::DynamicMatrix<float>
   * @param matrix
   * @param precision number of bits for each float 0 -max precision, 2 - 2
   * bita,32 - 32 bits
   * @return compressed data
   */
  ArchivedMatrix Compress(const blaze::DynamicMatrix<float>& matrix,
                          int precision);

  /**
   * Decompress a blaze::DynamicVector<float>
   * @param compressed compressed data
   * @return decompressed matrix
   */
  blaze::DynamicMatrix<float> Decompress(
      const ArchivedMatrix& compressed_matrix);

 private:
  /**
   * Compress indexes
   * @param indexes input integers
   * @param compressed output encoded data
   * @return compressed data size in bytes
   */
  size_t CompressIndexes(const std::vector<uint32_t>& indexes,
                         std::vector<uint8_t>* compressed);

  /**
   * Decompress indexes
   * @param compressed input encoded data
   * @param indexes output integers which must has length equal to original data
   * return read data size
   */
  size_t DecompressIndexes(const std::vector<uint8_t>& compressed,
                           std::vector<uint32_t>* indexes);

  /**
   * Compress values
   * @param values unput floats
   * @param compressed output encoded data
   * @param precision number of bits for each float
   * @return compressed data size in bytes
   */
  size_t CompressValues(const std::vector<float>& values,
                        std::vector<uint8_t>* compressed, int precision);

  /**
   * Decompress values
   * @param compressed input encoded data
   * @param values output floats which must has length equal to original data
   * return read data size
   */
  size_t DecompressValues(const std::vector<uint8_t>& compressed,
                          std::vector<float>* values);
};
}  // namespace drift::wavelet::internal
