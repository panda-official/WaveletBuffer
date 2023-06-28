// Copyright 2020-2023 PANDA GmbH

#include "internal/matrix_compressor.h"

#include <fpzip.h>
#include <streamvbyte.h>
#include <streamvbytedelta.h>

#include <iostream>

namespace drift::wavelet::internal {

std::tuple<std::vector<uint32_t>, std::vector<float>> ConvertToCSR(
    const blaze::DynamicMatrix<float>& matrix) {
  /* Check input */
  if (matrix.rows() == 0 || matrix.columns() == 0) {
    throw std::invalid_argument("Matrix is empty");
  }

  /* Indexes */
  std::vector<uint32_t> indexes;
  indexes.reserve(matrix.columns() * matrix.rows() / 2);

  /* Values */
  std::vector<float> values;
  values.reserve(matrix.columns() * matrix.rows() / 2);

  /* Fill indexes and value */
  for (auto i = 0; i < matrix.rows(); ++i) {
    for (auto j = 0; j < matrix.columns(); ++j) {
      if (matrix(i, j) != 0) {
        indexes.push_back(static_cast<uint32_t>(i * matrix.columns() + j));
        values.push_back(matrix(i, j));
      }
    }
  }

  return {indexes, values};
}

blaze::DynamicMatrix<float> ConvertFromCSR(const std::vector<uint32_t>& indexes,
                                           const std::vector<float>& values,
                                           const ArchivedMatrix& compressed) {
  blaze::DynamicMatrix<float> matrix(compressed.rows_number,
                                     compressed.cols_number, 0.0f);

  /* Fill */
  for (size_t i = 0; i < indexes.size(); ++i) {
    auto row = indexes[i] / compressed.cols_number;
    auto col = indexes[i] % compressed.cols_number;

    matrix(row, col) = values[i];
  }

  return matrix;
}

ArchivedMatrix BlazeCompressor::Compress(
    const blaze::DynamicMatrix<float>& matrix, int precision) {
  auto [indexes, values] = ConvertToCSR(matrix);

  ArchivedMatrix archived_matrix{
      true, values.size(), matrix.rows(), matrix.columns(), {}, {}};

  /* Compress indexes */
  CompressIndexes(indexes, &archived_matrix.indexes);

  /* Compress values */
  std::vector<uint8_t> compressed_values;
  CompressValues(values, &archived_matrix.values, precision);

  return archived_matrix;
}

blaze::DynamicMatrix<float> BlazeCompressor::Decompress(
    const ArchivedMatrix& compressed) {
  if (!compressed.is_valid) {
    throw std::invalid_argument("Invalid compressed matrix");
  }

  /* Decompress columns */
  std::vector<uint32_t> indexes;
  indexes.resize(compressed.nonzero);
  DecompressIndexes(compressed.indexes, &indexes);

  /* Decompress values */
  std::vector<float> values;
  values.resize(compressed.nonzero);
  DecompressValues(compressed.values, &values);

  /* Create matrix */
  auto matrix = ConvertFromCSR(indexes, values, compressed);

  return matrix;
}

size_t BlazeCompressor::CompressIndexes(const std::vector<uint32_t>& indexes,
                                        std::vector<uint8_t>* compressed) {
  compressed->resize(streamvbyte_max_compressedbytes(indexes.size()));

  /* Compress */
  size_t compressed_size =
      streamvbyte_delta_encode(indexes.data(), indexes.size(),
                               compressed->data(), 0) +
      STREAMVBYTE_PADDING;

  /* Trim */
  compressed->resize(compressed_size);

  // std::cout << "Compressed " << indexes.size() << " integers down to "
  //           << compressed_size << " bytes." << std::endl;

  return compressed_size;
}

size_t BlazeCompressor::DecompressIndexes(
    const std::vector<uint8_t>& compressed, std::vector<uint32_t>* indexes) {
  streamvbyte_delta_decode(compressed.data(), indexes->data(), indexes->size(),
                           0);
  return 0;
}

size_t BlazeCompressor::CompressValues(const std::vector<float>& values,
                                       std::vector<uint8_t>* compressed,
                                       int precision) {
  size_t N = values.size();
  size_t buffer_size = sizeof(float) * N + 1024;
  compressed->resize(buffer_size);

  FPZ* fpz = fpzip_write_to_buffer(reinterpret_cast<void*>(compressed->data()),
                                   compressed->size());

  fpz->type = FPZIP_TYPE_FLOAT;
  fpz->prec = precision;
  fpz->nx = N;
  fpz->ny = 1;
  fpz->nz = 1;
  fpz->nf = 1;

  size_t hs = fpzip_write_header(fpz);
  if (hs == 0) {
    fpzip_write_close(fpz);
    throw std::runtime_error(fpzip_errstr[fpzip_errno]);
  }

  size_t ds = fpzip_write(fpz, reinterpret_cast<const void*>(values.data()));
  if (ds == 0) {
    fpzip_write_close(fpz);
    throw std::runtime_error(fpzip_errstr[fpzip_errno]);
  }

  fpzip_write_close(fpz);

  compressed->resize(hs + ds);

  // std::cout << "Compressed " << N << " floats down to " << hs + ds << "
  // bytes."
  //           << std::endl;

  return hs + ds;
}

size_t BlazeCompressor::DecompressValues(const std::vector<uint8_t>& compressed,
                                         std::vector<float>* values) {
  FPZ* fpz =
      fpzip_read_from_buffer(reinterpret_cast<const void*>(compressed.data()));

  if (fpzip_read_header(fpz) == 0) {
    fpzip_read_close(fpz);
    throw std::runtime_error(fpzip_errstr[fpzip_errno]);
  }

  if (fpzip_read(fpz, reinterpret_cast<void*>(values->data())) == 0) {
    fpzip_read_close(fpz);
    throw std::runtime_error(fpzip_errstr[fpzip_errno]);
  }
  fpzip_read_close(fpz);

  return 0;
}

}  // namespace drift::wavelet::internal
