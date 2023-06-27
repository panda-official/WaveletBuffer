// Copyright 2020-2023 PANDA GmbH

#include "internal/matrix_compressor.h"

#include <iostream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

using drift::wavelet::internal::ArchivedMatrix;
using drift::wavelet::internal::BlazeCompressor;
/**
 * Random data generator
 **/
class DataGenerator {
  std::default_random_engine random_engine_;
  std::uniform_real_distribution<float> distribution_{0, 1};

 public:
  /**
   * Create sparse matrix with random data
   * @param rows matrix rows
   * @param cols matrix columns
   * @param ratio non-zero elements ratio
   * @return sparse matrix
   **/
  blaze::CompressedMatrix<float> GenerateSparseMatrix(size_t rows, size_t cols,
                                                      float ratio) {
    blaze::DynamicMatrix<float> matrix = blaze::zero<float>(rows, cols);
    for (size_t i = 0; i < rows; ++i) {
      for (size_t j = 0; j < cols; ++j) {
        if (distribution_(random_engine_) < ratio) {
          matrix(i, j) = distribution_(random_engine_);
        }
      }
    }
    return matrix;
  }
};

#ifndef _WIN32  // the tests hang on windows in debug mode

TEST_CASE("BlazeCompressor::Compress()", "[matrix]") {
  SECTION("Empty matrix") {
    auto compressor = BlazeCompressor();
    REQUIRE_THROWS_AS(compressor.Compress(blaze::CompressedMatrix<float>{}, 0),
                      std::invalid_argument);
  }

  SECTION("Random matrix") {
    DataGenerator generator;
    auto matrix = generator.GenerateSparseMatrix(100, 100, 0.1);
    auto compressed = BlazeCompressor().Compress(matrix, 0);

    REQUIRE(compressed.is_valid);
  }
}

TEST_CASE("BlazeCompressor::Decompress()", "[matrix]") {
  SECTION("Invalid compressed matrix") {
    ArchivedMatrix compressed;
    compressed.is_valid = false;
    auto bc = BlazeCompressor();
    REQUIRE_THROWS_AS(bc.Decompress(compressed), std::invalid_argument);
  }
  SECTION("Random matrix") {
    DataGenerator generator;
    auto matrix = generator.GenerateSparseMatrix(100, 100, 0.1);
    auto compressed = BlazeCompressor().Compress(matrix, 0);

    auto decompressed = BlazeCompressor().Decompress(compressed);
    REQUIRE(decompressed.rows() * decompressed.columns() > 0);
  }
}

TEST_CASE("Decompressed matrix must be equal to origin", "[matrix]") {
  DataGenerator generator;

  /* Define matrix parameters */
  auto rows = GENERATE(10, 100);
  auto columns = GENERATE(10, 100);
  auto ratio = GENERATE(0.0, 0.5, 1.0);

  /* Generate matrix */
  auto matrix = generator.GenerateSparseMatrix(rows, columns, ratio);

  /* Compress */
  auto compressed = BlazeCompressor().Compress(matrix, 0);
  REQUIRE(compressed.is_valid);

  /* Decompress */
  blaze::CompressedMatrix<float> decompressed =
      BlazeCompressor().Decompress(compressed);

  REQUIRE(matrix == decompressed);
}

TEST_CASE("Test custom precision", "[matrix]") {
  DataGenerator generator;
  int precision =
      GENERATE(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
               20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32);

  CAPTURE(precision);
  /* Generate matrix */
  auto matrix = generator.GenerateSparseMatrix(100, 100, 0.5);

  /* Compress */
  auto compressed = BlazeCompressor().Compress(matrix, precision);
  REQUIRE(compressed.is_valid);

  /* Decompress */
  blaze::CompressedMatrix<float> decompressed =
      BlazeCompressor().Decompress(compressed);

  auto norm = blaze::norm(matrix - decompressed);
  CAPTURE(norm);
  REQUIRE(norm < std::pow(2, 1 - precision) * matrix.nonZeros());
}

#endif
