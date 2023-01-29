// Copyright 2021-2022 PANDA GmbH

#include "wavelet_buffer/wavelet_buffer_serializer.h"

#include <matrix_compressor/matrix_compressor.h>
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
static bool ParseCompressedSubbands(blaze::Archive<std::istringstream>* archive,
                                    WaveletBuffer* buffer);

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
      if (!ParseCompressedSubbands(&archive, buffer.get())) {
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
    /* Load subbands */
    for (auto& signal : buffer->decompositions()) {
      for (auto& subband : signal) {
        if (sf_compression == 0) {
          archive >> subband;
        } else {
          matrix_compressor::CompressedMatrix data;
          data.is_valid = true;
          archive >> data.nonzero >> data.rows_number >> data.cols_number;

          blaze::DynamicVector<uint8_t> columns, rows, values;
          archive >> columns >> rows >> values;
          data.columns = std::vector<uint8_t>(columns.begin(), columns.end());
          data.rows = std::vector<uint8_t>(rows.begin(), rows.end());
          data.values = std::vector<uint8_t>(values.begin(), values.end());

          subband = matrix_compressor::BlazeCompressor().Decompress(data);
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

  try {
    blaze::Archive arch(ss);

    sf_compression = std::min<uint8_t>(16, sf_compression);
    if (buffer.IsEmpty()) {
      sf_compression = 0;
    }

    /* Serialize header */
    arch << kSerializationVersion << buffer.parameters() << sf_compression;

    /* Serialize subbands */
    for (const auto& signal : buffer.decompositions()) {
      for (const auto& subband : signal) {
        if (sf_compression == 0) {
          arch << subband;
        } else {
          auto data = matrix_compressor::BlazeCompressor().Compress(subband);
          if (!data.is_valid) {
            return false;
          }

          /* Serialize compressed data */
          arch << data.nonzero;
          arch << data.rows_number;
          arch << data.cols_number;
          arch << blaze::DynamicVector<uint8_t>(data.columns.size(),
                                                data.columns.data());
          arch << blaze::DynamicVector<uint8_t>(data.rows.size(),
                                                data.rows.data());
          arch << blaze::DynamicVector<uint8_t>(data.values.size(),
                                                data.values.data());
        }
      }
    }

    *blob = ss.str();

    return true;

  } catch (std::exception& e) {
    std::cerr << "Failed serialize data: " << e.what() << std::endl;
    return false;
  }
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

bool ParseCompressedSubbands(blaze::Archive<std::istringstream>* archive,
                             WaveletBuffer* buffer) {
  /* Iterate through sabbands */
  for (int n = 0; n < buffer->parameters().signal_number; ++n) {
    for (int s = 0; s < buffer->decompositions()[n].size(); ++s) {
      blaze::DynamicVector<uint8_t> compressed_subband;
      *archive >> compressed_subband;

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
