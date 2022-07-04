// Copyright 2020 PANDA GmbH
#ifndef WAVELET_BUFFER_WAVELET_PARAMETERS_H_
#define WAVELET_BUFFER_WAVELET_PARAMETERS_H_

#include <blaze/Blaze.h>
#include <cstddef>

#include <algorithm>
#include <ostream>
#include <vector>

namespace drift {

/**
 * Different types of wavelet function
 */
enum WaveletTypes {
  kNone = 0,  // No composition and no padding
  kDB1 = 1,
  kDB2 = 2,
  kDB3 = 3,
  kDB4 = 4,
  kDB5 = 5,
};

/**
 * Signal shape, order from low to high (width, height, color, etc)
 */
using SignalShape = std::vector<size_t>;

/**
 * Parameters of wavelet decomposition
 */
struct WaveletParameters {
  SignalShape signal_shape;
  size_t signal_number;        // Channels in the signal (RGB=3)
  size_t decomposition_steps;  // Steps fo the decompositions
  WaveletTypes wavelet_type;
  bool is_raw_convolve_1d{true};  // Only for develop purposes, don't change it

  bool operator==(const WaveletParameters& rhs) const {
    return signal_shape == rhs.signal_shape &&
           signal_number == rhs.signal_number &&
           decomposition_steps == rhs.decomposition_steps &&
           wavelet_type == rhs.wavelet_type;
  }

  bool operator!=(const WaveletParameters& rhs) const {
    return !(rhs == *this);
  }

  bool operator<(const WaveletParameters& rhs) const {
    return std::tie(signal_shape, signal_number, decomposition_steps,
                    wavelet_type) <
           std::tie(rhs.signal_shape, rhs.signal_number,
                    rhs.decomposition_steps, rhs.wavelet_type);
  }

  bool operator>(const WaveletParameters& rhs) const { return rhs < *this; }
  bool operator<=(const WaveletParameters& rhs) const { return !(rhs < *this); }
  bool operator>=(const WaveletParameters& rhs) const { return !(*this < rhs); }

  [[nodiscard]] size_t dimension() const { return signal_shape.size(); }

  [[nodiscard]] size_t shortest_dimension() const {
    return *std::min_element(std::begin(signal_shape), std::end(signal_shape));
  }

  /**
   * Cereal save interface
   * @tparam Archive
   * @param ar
   */
  template <class Archive>
  void save(Archive& ar) const {  // NOLINT
    ar(signal_shape, signal_number, decomposition_steps, wavelet_type);
  }

  /**
   * Cereal load interface
   * @tparam Archive
   * @param ar
   */
  template <class Archive>
  void load(Archive& ar) {  // NOLINT
    ar(signal_shape, signal_number, decomposition_steps, wavelet_type);
  }

  friend std::ostream& operator<<(std::ostream& os,
                                  const WaveletParameters& parameters) {
    os << "WaveletParameters {" << std::endl
       << "  signal_shape: "
       << blaze::trans(blaze::DynamicVector<size_t>(
              parameters.signal_shape.size(), parameters.signal_shape.data()))
       << "  signal_number: " << parameters.signal_number << std::endl
       << "  decomposition_steps: " << parameters.decomposition_steps
       << std::endl
       << "  wavelet_type: " << parameters.wavelet_type << std::endl
       << "}";
    return os;
  }
};

/**
 * Blaze serializer for vector
 * @tparam Archive
 * @tparam T
 * @param archive
 * @param p
 */
template <typename Archive>
void serialize(Archive& archive, const SignalShape& p) {  // NOLINT
  archive << p.size();
  for (const auto& el : p) {
    archive << el;
  }
}

/**
 * Blaze de-serializer for vector
 * @tparam Archive
 * @tparam T
 * @param archive
 * @param p
 */
template <typename Archive>
void deserialize(Archive& archive, SignalShape& p) {  // NOLINT
  size_t size;
  archive >> size;
  p.resize(size);

  for (size_t i = 0; i < size; i++) {
    archive >> p[i];
  }
}

/**
 * Blaze serializer
 * @tparam Archive
 * @param archive
 * @param p
 */
template <typename Archive>
void serialize(Archive& archive, const WaveletParameters& p) {  // NOLINT
  serialize(archive, p.signal_shape);
  archive << p.signal_number << p.decomposition_steps << int(p.wavelet_type);
}

/**
 * Blaze de-serializer
 * @tparam Archive
 * @param archive
 * @param p
 */
template <typename Archive>
void deserialize(Archive& archive, WaveletParameters& p) {  // NOLINT
  int wavelet_type;
  deserialize(archive, p.signal_shape);
  archive >> p.signal_number >> p.decomposition_steps >> wavelet_type;
  p.wavelet_type = WaveletTypes(wavelet_type);
}

}  // namespace drift

#endif  // WAVELET_BUFFER_WAVELET_PARAMETERS_H_
