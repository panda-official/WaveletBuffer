// Copyright 2021-2022 PANDA GmbH

#include "wavelet_buffer/wavelet_buffer_serializer.h"

#include <sf_compressor/sf_compressor.h>

#include <iostream>

#include "wavelet_buffer/wavelet_buffer.h"

namespace drift {
/**
 * We need to allocate data for SfComprressor depending on subband size
 * @param signal_shape
 * @param sub_number number of suuband, if 0 it is the original signal
 * @return needed memory size of subband in bytes + 50% reserve
 */
static size_t GetMemorySizeForSfCompressor(const SignalShape& signal_shape,
                                           int sub_number = 0);

/**
 * Parse blaze archive and fill buffer decompositions in place
 * @param archive blaze archive with sabbands
 * @param buffer WaveletBuffer
 * @return success
 */
static bool ParseCompressedSubbands(blaze::Archive<std::istringstream>& archive,
                                    std::unique_ptr<WaveletBuffer>& buffer);

[[nodiscard]] std::unique_ptr<WaveletBuffer>
WaveletBufferSerializerLegacy::Parse(const std::string& blob) {
  try {
    /* Create archive */
    std::istringstream ss(blob);
    blaze::Archive archive(ss);

    /* Load version and parameters */
    uint8_t serialization_version;
    WaveletParameters params{};
    uint8_t sf_compression;
    archive >> serialization_version >> params >> sf_compression;

    auto buffer = std::make_unique<WaveletBuffer>(params);

    if (sf_compression != 0) {
      if (!ParseCompressedSubbands(archive, buffer)) {
        return nullptr;
      }
    } else {
      archive >> buffer->decompositions();
    }

    return buffer;

  } catch (std::exception& e) {
    std::cerr << "Failed parse data: " << e.what() << std::endl;
    return nullptr;
  }
}

[[nodiscard]] bool WaveletBufferSerializerLegacy::Serialize(
    const WaveletBuffer& buffer, std::string* blob, uint8_t sf_compression) {
  return false;
}

[[nodiscard]] std::unique_ptr<WaveletBuffer> WaveletBufferSerializer::Parse(
    const std::string& blob) {
  try {
    /* Create archive */
    std::istringstream ss(blob);
    blaze::Archive archive(ss);

    /* Load version and parameters */
    uint8_t serialization_version;
    WaveletParameters params{};
    uint8_t sf_compression;
    archive >> serialization_version >> params >> sf_compression;

    auto buffer = std::make_unique<WaveletBuffer>(params);
    if (sf_compression != 0) {
      if (!ParseCompressedSubbands(archive, buffer)) {
        return nullptr;
      }
    } else {
      for (auto& signal : buffer->decompositions()) {
        for (auto& subband : signal) {
          archive >> subband;
        }
      }
    }
    return buffer;
  } catch (std::exception& e) {
    std::cerr << "Failed parse data: " << e.what() << std::endl;
    return nullptr;
  }
}

[[nodiscard]] bool WaveletBufferSerializer::Serialize(
    const WaveletBuffer& buffer, std::string* blob, uint8_t sf_compression) {
  std::stringstream ss;
  {
    blaze::Archive arch(ss);

    sf_compression = std::min<uint8_t>(16, sf_compression);
    if (buffer.IsEmpty()) {
      sf_compression = 0;
    }

    arch << kSerializationVersion << buffer.parameters() << sf_compression;

    if (sf_compression == 0) {
      for (const auto& signal : buffer.decompositions()) {
        for (const auto& subband : signal) {
          arch << subband;
        }
      }
    } else {
      uint8_t frag_len;
      if (sf_compression == 1) {
        frag_len = 23;
      } else {
        frag_len = 23 - sf_compression;
      }

      size_t expected_points =
          GetMemorySizeForSfCompressor(buffer.parameters().signal_shape);
      sf::SfCompressor compressor(expected_points);

      /* Iterate through sabband */
      for (int n = 0; n < buffer.parameters().signal_number; ++n) {
        for (const auto& subband : buffer.decompositions()[n]) {
          /* Prepare input data */
          sf::SfCompressor::OriginalData in_data = {
              .frag_length = frag_len,
              .row_based = true,
              .rows = subband.rows(),
              .columns = subband.columns(),
              .indexes{},
              .values{},
          };

          in_data.indexes.reserve(subband.nonZeros());
          in_data.values.reserve(subband.nonZeros());

          blaze::CompressedMatrix<DataType> compressed_matrix = subband;

          for (int i = 0; i < compressed_matrix.rows(); ++i) {
            const auto base_index = i * subband.columns();
            for (auto it = compressed_matrix.begin(i);
                 it != compressed_matrix.end(i); ++it) {
              in_data.values.push_back(it->value());
              in_data.indexes.push_back(base_index + it->index());
            }
          }

          /* Compress data */
          std::vector<uint8_t> compressed_subband;
          if (!compressor.Compress(in_data, &compressed_subband)) {
            return false;
          }

          /* Serialize data */
          arch << blaze::DynamicVector<uint8_t>(compressed_subband.size(),
                                                compressed_subband.data());
        }
      }
    }
  }
  *blob = ss.str();

  return true;
}

size_t GetMemorySizeForSfCompressor(const SignalShape& signal_shape,
                                    int sub_number) {
  const bool is_one_dim = signal_shape.size() == 1;
  const auto max_size =
      (is_one_dim ? signal_shape[0] : signal_shape[0] * signal_shape[1]) *
      sizeof(DataType);
  const auto divisor =
      is_one_dim ? std::pow(2, sub_number) : std::pow(4, sub_number / 3);
  return static_cast<size_t>(static_cast<double>(max_size) / divisor * 1.5);
}

bool ParseCompressedSubbands(blaze::Archive<std::istringstream>& archive,
                             std::unique_ptr<WaveletBuffer>& buffer) {
  /* Iterate through sabbands */
  for (int n = 0; n < buffer->parameters().signal_number; ++n) {
    for (int s = 0; s < buffer->decompositions()[n].size(); ++s) {
      blaze::DynamicVector<uint8_t> compressed_subband;
      archive >> compressed_subband;

      std::vector<uint8_t> data(compressed_subband.begin(),
                                compressed_subband.end());

      size_t expected_points =
          GetMemorySizeForSfCompressor(buffer->parameters().signal_shape, s);
      sf::SfCompressor compressor(
          expected_points);  // TODO(Alexey Timin): Should
                             // reallocate memory because of a
                             // bug in SfCompressor

      sf::SfCompressor::OriginalData out_data;
      if (!compressor.Decompress(data, &out_data)) {
        return false;
      }

      auto subband = Subband(out_data.rows, out_data.columns, 0);
      for (int i = 0; i < out_data.indexes.size(); ++i) {
        subband(out_data.indexes[i] / out_data.columns,
                out_data.indexes[i] % out_data.columns) = out_data.values[i];
      }
      buffer->decompositions()[n][s] = std::move(subband);
    }
  }
  return true;
}
}  // namespace drift
