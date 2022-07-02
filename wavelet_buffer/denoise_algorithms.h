// Copyright 2020-2021 PANDA GmbH
#ifndef WAVELET_BUFFER_DENOISE_ALGORITHMS_H_
#define WAVELET_BUFFER_DENOISE_ALGORITHMS_H_

#include <blaze/Blaze.h>

#include <algorithm>
#include <tuple>

namespace drift {
/**
 * Interface for different algorithms to reduce the noise in subbands
 * @tparam T
 */
template <typename T>
class DenoiseAlgorithm {
 public:
  using Signal2D = blaze::DynamicMatrix<T>;
  using Signal1D = blaze::DynamicVector<T>;
  virtual ~DenoiseAlgorithm() = default;

  /**
   * Remove noise from input signal
   * @param data the input signal
   * @step denoise step number
   * @return return "clean" signal
   */
  virtual Signal1D Denoise(const Signal1D &data,
                           const size_t step = 0) const = 0;
  virtual Signal2D Denoise(const Signal2D &data,
                           const size_t step = 0) const = 0;
};

/**
 * Does nothing
 * @tparam T
 */
template <typename T>
class NullDenoiseAlgorithm : public DenoiseAlgorithm<T> {
 public:
  using Signal1D = typename DenoiseAlgorithm<T>::Signal1D;
  using Signal2D = typename DenoiseAlgorithm<T>::Signal2D;

  Signal1D Denoise(const Signal1D &data, const size_t step = 0) const override {
    return data;
  }
  Signal2D Denoise(const Signal2D &data, const size_t step = 0) const override {
    return data;
  }
};

/**
 * Set to zero values which absolute value less than threshold computed by `A *
 * step + b`
 * @tparam T
 */
template <typename T>
class ThresholdAbsDenoiseAlgorithm : public DenoiseAlgorithm<T> {
 public:
  using Signal1D = typename DenoiseAlgorithm<T>::Signal1D;
  using Signal2D = typename DenoiseAlgorithm<T>::Signal2D;

  /**
   * Constructor
   * @param a is `A`
   * @param b is `B`
   */
  explicit ThresholdAbsDenoiseAlgorithm(T a, T b) : a_(a), b_(b) {}

  /**
   * Denoise 2D data
   * @param data input data
   * @param step denoise step
   * @return denoised data
   */
  Signal2D Denoise(const Signal2D &data, const size_t step = 0) const override {
    const T threshold = GetThreshold(step);

    Signal2D result = blaze::zero<T>(data.rows(), data.columns());

    for (size_t i = 0UL; i < data.rows(); ++i) {
      for (size_t j = 0UL; j < data.columns(); ++j) {
        if (std::abs(data(i, j)) > threshold) {
          result(i, j) = data(i, j);
        }
      }
    }

    return result;
  }

  /**
   * Denoise 1D data
   * @param data input data
   * @param step denoise step
   * @return denoised data
   */
  Signal1D Denoise(const Signal1D &data, const size_t step = 0) const override {
    const T threshold = GetThreshold(step);

    Signal1D result = blaze::zero<T>(data.size());

    for (size_t i = 0; i < data.size(); ++i) {
      if (std::abs(data[i]) > threshold) {
        result[i] = data[i];
      }
    }
    return result;
  }

 private:
  T GetThreshold(const size_t step) const { return a_ * step + b_; }

  T a_;
  T b_;
};

/**
 * Suppresses a part of the smallest values in subbands
 * @tparam T
 */
template <typename T>
class SimpleDenoiseAlgorithm : public DenoiseAlgorithm<T> {
 public:
  using Signal1D = typename DenoiseAlgorithm<T>::Signal1D;
  using Signal2D = typename DenoiseAlgorithm<T>::Signal2D;

  /**
   * Create a denoiser
   * @param compression_level the level of the compression 0 - no compression, 1
   * - all values are zero
   */
  explicit SimpleDenoiseAlgorithm(T compression_level)
      : compression_level_(compression_level) {}

  Signal2D Denoise(const Signal2D &data, const size_t step = 0) const override {
    // Save all abs values and coordinates to sort by their absolute values
    blaze::DynamicVector<std::tuple<T, size_t, size_t>> values(
        blaze::size(data));
    for (size_t i = 0; i < data.rows(); ++i) {
      for (size_t j = 0; j < data.columns(); ++j) {
        values[i + j * data.rows()] = {std::fabs(data(i, j)), i, j};
      }
    }

    // substract vom size to ensure an identical split as the original algorithm
    // due to different rounding
    const auto slice_index =
        values.size() - (std::min<size_t>(values.size() * compression_level_,
                                          values.size() - 1));

    // absolute sorting is not needed, just partion the data sufficient
    std::nth_element(
        values.begin(), values.begin() + slice_index, values.end(),
        [](auto a, auto b) { return std::get<0>(a) > std::get<0>(b); });

    // Slice the sorted values on the compression level and copy only the
    // biggest values
    Signal2D result(data.rows(), data.columns(), 0);

    for (size_t c = 0; c < slice_index; ++c) {
      auto [tr, i, j] = values[c];
      result(i, j) = data(i, j);
    }

    return result;
  }

  /**
   * @param data
   * @return
   */
  Signal1D Denoise(const Signal1D &data, const size_t step = 0) const override {
    // Save all abs values and coordinates to sort by their absolute values
    blaze::DynamicVector<std::tuple<T, size_t>> values(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
      values[i] = {std::fabs(data[i]), i};
    }

    // substract vom size to ensure an identical split as the original algorithm
    // due to different rounding
    const auto slice_index =
        values.size() - (std::min<size_t>(values.size() * compression_level_,
                                          values.size() - 1));

    // absolute sorting is not needed, just partion the data sufficient
    std::nth_element(
        values.begin(), values.begin() + slice_index, values.end(),
        [](auto a, auto b) { return std::get<0>(a) > std::get<0>(b); });

    // Slice the sorted values on the compression level and copy only the
    // biggest values

    Signal1D result(data.size(), 0);

    for (size_t c = 0; c < slice_index; ++c) {
      auto [tr, i] = values[c];
      result[i] = data[i];
    }

    return result;
  }

 private:
  T compression_level_;
};

}  // namespace drift

#endif  // WAVELET_BUFFER_DENOISE_ALGORITHMS_H_
