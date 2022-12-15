// Copyright 2020-2021 PANDA GmbH

#include "wavelet_buffer/wavelet_buffer.h"

#include <blaze/Blaze.h>

#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "wavelet_buffer/wavelet_buffer_serializer.h"
#include "wavelet_buffer/wavelet_buffer_view.h"

namespace drift {

/**
 * Encapsulate implementation details of WaveletBuffer
 */
class WaveletBuffer::Impl {
 public:
  /**
   * Initialize buffer
   * @param parameters the parameters of the wavelet decomposition
   */
  explicit Impl(WaveletParameters parameters)
      : parameters_(std::move(parameters)) {
    if (parameters_.dimension() != 2 && parameters_.dimension() != 1) {
      throw std::runtime_error("Only 1D & 2D decomposition is supported");
    }

    auto shortest_dimension = [](auto& signal_shape) {
      return *std::min_element(std::begin(signal_shape),
                               std::end(signal_shape));
    };

    const auto min_signal_size = CalculateMinSignalSize();
    if (shortest_dimension(parameters_.signal_shape) < min_signal_size) {
      throw std::runtime_error("Input signal shape is too small");
    }

    const auto max_decomposition_steps =
        internal::CalculateMaxDecompositionSteps(parameters_.wavelet_type,
                                                 parameters_.signal_shape);
    if (parameters_.decomposition_steps > max_decomposition_steps) {
      throw std::runtime_error(std::string("Too many decomposition steps for "
                                           "this signal size with that wavelet "
                                           "type (must be max ") +
                               std::to_string(max_decomposition_steps) + ").");
    }

    parameters_.decomposition_steps =
        parameters_.wavelet_type != WaveletTypes::kNone
            ? parameters_.decomposition_steps
            : 0;

    decompositions_.resize(parameters_.signal_number);
    const auto decomposition_size = DecompositionSize(parameters_);

    for (auto& signal : decompositions_) {
      signal.resize(decomposition_size);
    }
  }

  Impl(WaveletParameters parameters, NWaveletDecomposition decompositions)
      : Impl(std::move(parameters)) {
    if (parameters_.signal_number != decompositions.size()) {
      throw std::runtime_error(
          "Wrong signal number in decomposition. Expected " +
          std::to_string(parameters_.signal_number) + " but got " +
          std::to_string(decompositions.size()));
    }

    for (int n = 0; n < decompositions_.size(); ++n) {
      const auto expected_size = decompositions_[n].size();
      const auto received_size = decompositions[n].size();
      if (expected_size != received_size) {
        throw std::runtime_error("Wrong number of subbands in signal " +
                                 std::to_string(n) + ". Expected " +
                                 std::to_string(expected_size) + " but got " +
                                 std::to_string(received_size));
      }
    }

    decompositions_ = std::move(decompositions);
  }
  /**** Wavelet transformation ********/

  /**
   * Decomposes the signal into the subbands and saves it internally
   * @param data the signal
   * @return true if it has no errors
   */
  bool Decompose(const SignalN2D& data,
                 const DenoiseAlgorithm<DataType>& denoiser) {
    return internal::DecomposeImpl(parameters_, &decompositions_, data,
                                   denoiser, 0, parameters_.signal_number);
  }

  /**
   * Decomposes the signal into the subbands and saves it internally
   * @param data the signal
   * @return true if it has no errors
   */
  bool Decompose(const Signal1D& data,
                 const DenoiseAlgorithm<DataType>& denoiser) {
    SignalN2D data2d = {Signal2D(data.size(), 1)};
    blaze::column(data2d[0], 0) = data;

    return internal::DecomposeImpl(parameters_, &decompositions_, data2d,
                                   denoiser, 0, 1);
  }

  /**
   * Composes the internal subbands into a signal
   * @param data the signal
   * @return true if it has no errors
   */
  bool Compose(SignalN2D* data, int scale_factor) const {
    return internal::ComposeImpl(parameters_, data, decompositions_,
                                 scale_factor, 0, parameters_.signal_number);
  }

  /**
   * Composes the internal subbands into a signal
   * @param data the signal
   * @return true if it has no errors
   */
  bool Compose(Signal1D* data, int scale_factor) const {
    SignalN2D data2d;
    auto ret =
        internal::ComposeImpl(parameters_, &data2d, decompositions_,
                              scale_factor, 0, parameters_.signal_number);
    if (ret) {
      *data = blaze::column(data2d[0], 0);
    }
    return true;
  }

  /******** Serializers **************/

  /**
   * Parses subbands from a blob of data and creates a new buffer
   * @param blob the blob of subbands
   * @return nullptr if it failed to parse the buffer
   */
  [[nodiscard]] static std::unique_ptr<WaveletBuffer> Parse(
      const std::string& blob) {
    /* Read binary version */
    std::istringstream ss(blob);
    uint8_t version;
    ss >> version;

    std::unique_ptr<IWaveletBufferSerializer> serializer;

    /* Choose serializer */
    if (version == kSerializationVersion) {
      serializer = std::make_unique<WaveletBufferSerializer>();
    } else if (version == 2) {
      serializer = std::make_unique<WaveletBufferSerializerLegacy>();
    } else {
      std::cerr << "Wrong version of binary: It is "
                << static_cast<int>(version) << " but must be "
                << static_cast<int>(kSerializationVersion) << std::endl;
      return nullptr;
    }

    return serializer->Parse(blob);
  }
  /**
   * Serialize the buffer into the blob for saving in a file or sending via
   * network
   * @param blob the blob to serialize
   * @param sf_compression - 0 - switch off, 16 - max compression(bfloat).
   * @return return true if it has no error
   */
  [[nodiscard]] bool Serialize(std::string* blob,
                               uint8_t sf_compression = 0) const {
    return WaveletBufferSerializer().Serialize({parameters_, decompositions_},
                                               blob, sf_compression);
  }

  /******** Accessors ***************/

  /**
   * Access to the decomposition by channels
   * NOTE: the last element is always an abstraction
   * @param index the index of the channel
   * @return the decomposition as a list of the subbands
   */
  [[nodiscard]] WaveletDecomposition& operator[](int index) {
    return decompositions_[index];
  }

  /**
   * Const access to the decomposition by channels
   * @param index the index of the channel
   * @return the decomposition as a list of the subbands
   */
  [[nodiscard]] const WaveletDecomposition& operator[](int index) const {
    return decompositions_[index];
  }

  /**
   * Parameters of the decomposition
   */
  [[nodiscard]] const WaveletParameters& parameters() const {
    return parameters_;
  }

  [[nodiscard]] blaze::DynamicVector<WaveletDecomposition>& decompositions() {
    return decompositions_;
  }

  [[nodiscard]] const blaze::DynamicVector<WaveletDecomposition>&
  decompositions() const {
    return decompositions_;
  }

  bool operator==(const Impl& rhs) const {
    return parameters_ == rhs.parameters_ &&
           decompositions_ == rhs.decompositions_;
  }

  bool operator!=(const Impl& rhs) const { return rhs != *this; }

  /******** Helpers ****************/

  /**
   * @brief Calculate the minimum signal size
   *
   * @return minimum length of each dimension
   */
  [[nodiscard]] int CalculateMinSignalSize() const {
    const int type = static_cast<int>(parameters_.wavelet_type);
    return type * 2;
  }

  /**
   * Get the size of all subbands in bytes
   * Note: this doesn't take into account size of container, only pixels
   */
  [[nodiscard]] size_t CompressedSize() const {
    auto calc_non_zeros = [](size_t a, const auto& dec) -> size_t {
      return std::accumulate(dec.begin(), dec.end(), a,
                             [](size_t b, const auto& subband) -> size_t {
                               return b + subband.nonZeros() * sizeof(DataType);
                             });
    };

    return std::accumulate(decompositions_.begin(), decompositions_.end(), 0,
                           calc_non_zeros);
  }

  /**
   * Gets min and max factors of values in subband
   * example:
   *    if your signal has values in interval 0..5, so the pair (-2, 2)
   * means that you a subband has values in interval -10..10
   * @param index the index of the subband
   * @return a pair of min and max factors
   */
  [[nodiscard]] std::pair<DataType, DataType> GetValueRange(
      size_t index) const {
    assert(index < decompositions_[0].size() && "index is out of range");
    bool is_approximation = index == (decompositions_[0].size() - 1);

    DataType delta = std::pow(2, parameters_.decomposition_steps - 1);
    if (!is_approximation) {
      delta = std::pow(
          2, index / internal::SubbandsPerWaveletTransform(parameters_));
    }

    return is_approximation ? std::make_pair(DataType{}, 2 * delta)
                            : std::make_pair(-delta, delta);
  }

  [[nodiscard]] bool IsEmpty() const {
    return std::all_of(
        decompositions_.begin(), decompositions_.end(), [](const auto& deca) {
          return std::all_of(deca.begin(), deca.end(), [](const auto& mtx) {
            return mtx.rows() * mtx.columns() == 0;
          });
        });
  }

 private:
  WaveletParameters parameters_;

  /* Channel -> subbands (vector of all details and last approx in the end)
   */
  blaze::DynamicVector<WaveletDecomposition> decompositions_;
};

/** Wavelet constructors and destructor **/

WaveletBuffer::~WaveletBuffer() = default;

WaveletBuffer::WaveletBuffer(WaveletBuffer&& buffer) noexcept = default;

WaveletBuffer::WaveletBuffer(const WaveletParameters& parameters)
    : impl_(std::make_unique<Impl>(parameters)) {}

WaveletBuffer::WaveletBuffer(const WaveletParameters& parameters,
                             const NWaveletDecomposition& decompositions)
    : impl_(std::make_unique<Impl>(parameters, decompositions)) {}

WaveletBuffer::WaveletBuffer(const WaveletBuffer& buffer)
    : impl_(std::make_unique<Impl>(*buffer.impl_)) {}

/**** Wavelet transformation ********/
bool WaveletBuffer::Decompose(const SignalN2D& data,
                              const DenoiseAlgorithm<DataType>& denoiser) {
  return impl_->Decompose(data, denoiser);
}

bool WaveletBuffer::Decompose(const Signal1D& data,
                              const DenoiseAlgorithm<DataType>& denoiser) {
  return impl_->Decompose(data, denoiser);
}

bool WaveletBuffer::Compose(SignalN2D* data, int scale_factor) const {
  return impl_->Compose(data, scale_factor);
}

bool WaveletBuffer::Compose(Signal1D* data, int scale_factor) const {
  return impl_->Compose(data, scale_factor);
}

/******** Serializers **************/

[[nodiscard]] std::unique_ptr<WaveletBuffer> WaveletBuffer::Parse(
    const std::string& blob) {
  return Impl::Parse(blob);
}

[[nodiscard]] bool WaveletBuffer::Serialize(std::string* blob,
                                            uint8_t sf_compression) const {
  return impl_->Serialize(blob, sf_compression);
}

/******** Accessors ***************/

[[nodiscard]] WaveletDecomposition& WaveletBuffer::operator[](int index) {
  return impl_->operator[](index);
}

[[nodiscard]] const WaveletDecomposition& WaveletBuffer::operator[](
    int index) const {
  return impl_->operator[](index);
}

const WaveletParameters& WaveletBuffer::parameters() const {
  return impl_->parameters();
}

NWaveletDecomposition& WaveletBuffer::decompositions() {
  return impl_->decompositions();
}

const NWaveletDecomposition& WaveletBuffer::decompositions() const {
  return impl_->decompositions();
}

bool WaveletBuffer::operator==(const WaveletBuffer& rhs) const {
  return *impl_ == *(rhs.impl_);
}

bool WaveletBuffer::operator!=(const WaveletBuffer& rhs) const {
  return rhs != *this;
}

WaveletBuffer& WaveletBuffer::operator=(const WaveletBuffer& buffer) {
  impl_ = std::make_unique<Impl>(*buffer.impl_);
  return *this;
}

WaveletBuffer& WaveletBuffer::operator=(WaveletBuffer&& buffer) noexcept =
    default;

/******** Helpers ****************/
std::pair<DataType, DataType> WaveletBuffer::GetValueRange(size_t index) const {
  return impl_->GetValueRange(index);
}

WaveletBufferView WaveletBuffer::operator()(int index, int count) {
  return {index, count, this};
}

WaveletBufferView WaveletBuffer::operator()(int index, int count) const {
  return {index, count, this};
}

bool WaveletBuffer::IsEmpty() const { return impl_->IsEmpty(); }

std::ostream& operator<<(std::ostream& os, const WaveletBuffer& wb) {
  const auto& d = wb.decompositions();

  os << "Wavelet Buffer {" << std::endl
     << wb.parameters() << std::endl
     << "WaveletDecomposition {" << std::endl;

  size_t chn = 0;
  for (const auto& ch : d) {
    std::cout << "  channel: " << std::to_string(chn++) << std::endl;

    for (const auto& sb : ch) {
      // TODO(victor1234): add support for 2d subbands
      os << "  " << blaze::trans(sb);
    }
  }
  os << "}" << std::endl << "}" << std::endl;

  return os;
}

}  // namespace drift
