// Copyright 2021-2023 PANDA GmbH
#ifndef WAVELET_BUFFER_INTERNAL_SF_COMPRESSOR_H_
#define WAVELET_BUFFER_INTERNAL_SF_COMPRESSOR_H_

#include <memory>
#include <vector>

namespace drift::wavelet::internal {

/**
 * Sparse float compressor
 * Deprecated, used for compatibility with old data
 */
class SfCompressor {
 public:
  /**
   * Input
   */
  struct OriginalData {
    uint8_t frag_length;  // 7-bfloat, 23 -float, possible values 7-21 and 23
    bool row_based;
    size_t rows;
    size_t columns;
    std::vector<uint32_t> indexes;
    std::vector<float> values;

    bool operator==(const OriginalData &rhs) const;
    bool operator!=(const OriginalData &rhs) const;
  };

  /**
   * Create a compressor with a buffer for 1M points
   */
  SfCompressor() : SfCompressor(5'000'000) {}

  /**
   * Create compressor
   * @param buffer_size size of internal buffer,
   * must be ~5*expected points for float and 2.5*expected points for bfloat
   */
  explicit SfCompressor(size_t buffer_size);

  ~SfCompressor();

  [[nodiscard]] bool Compress(const OriginalData &origin,
                              std::vector<uint8_t> *blob) const;

  bool Decompress(const std::vector<uint8_t> &blob, OriginalData *origin) const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace drift::wavelet::internal

#endif  // WAVELET_BUFFER_INTERNAL_SF_COMPRESSOR_H_
