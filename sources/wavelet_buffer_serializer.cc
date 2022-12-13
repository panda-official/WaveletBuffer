#include "wavelet_buffer/wavelet_buffer_serializer.h"

#include <sf_compressor/sf_compressor.h>

#include "wavelet_buffer/wavelet_buffer.h"

namespace drift {
[[nodiscard]] std::unique_ptr<WaveletBuffer> WaveletBufferSerializer::Parse(
    const std::string& blob) {
  try {
    std::unique_ptr<WaveletBuffer> buffer;

    std::istringstream ss(blob);
    blaze::Archive blaze_arch(ss);

    uint8_t serialization_version;
    WaveletParameters params{};
    uint8_t sf_compression;
    blaze_arch >> serialization_version >> params >> sf_compression;

    buffer = std::make_unique<WaveletBuffer>(params);

    if (serialization_version != kSerializationVersion) {
      std::cerr << "Wrong version of binary: It is "
                << static_cast<int>(serialization_version) << " but must be "
                << static_cast<int>(kSerializationVersion) << std::endl;
      return nullptr;
    } else {
      if (sf_compression != 0) {
        for (int n = 0; n < params.signal_number; ++n) {
          for (int s = 0; s < buffer->decompositions()[n].size(); ++s) {
            blaze::DynamicVector<uint8_t> compressed_subband;
            blaze_arch >> compressed_subband;

            std::vector<uint8_t> data(compressed_subband.begin(),
                                      compressed_subband.end());

            size_t expected_points =
                GetMemorySizeForSfCompressor(params.signal_shape, s);
            sf::SfCompressor compressor(
                expected_points);  // TODO(Alexey Timin): Should
                                   // reallocate memory because of a
                                   // bug in SfCompressor
            sf::SfCompressor::OriginalData out_data;
            if (!compressor.Decompress(data, &out_data)) {
              return nullptr;
            }

            auto subband = Subband(out_data.rows, out_data.columns, 0);
            for (int i = 0; i < out_data.indexes.size(); ++i) {
              subband(out_data.indexes[i] / out_data.columns,
                      out_data.indexes[i] % out_data.columns) =
                  out_data.values[i];
            }
            buffer->decompositions()[n][s] = std::move(subband);
          }
        }
      } else {
        blaze_arch >> buffer->decompositions();
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
      arch << buffer.decompositions();
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
      for (int n = 0; n < buffer.parameters().signal_number; ++n) {
        for (const auto& subband : buffer.decompositions()[n]) {
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

          std::vector<uint8_t> compressed_subband;
          if (!compressor.Compress(in_data, &compressed_subband)) {
            return false;
          }

          arch << blaze::DynamicVector<uint8_t>(compressed_subband.size(),
                                                compressed_subband.data());
        }
      }
    }
  }
  *blob = ss.str();

  return true;
}

size_t WaveletBufferSerializer::GetMemorySizeForSfCompressor(
    const SignalShape& signal_shape, int sub_number) {
  const bool is_one_dim = signal_shape.size() == 1;
  const auto max_size =
      (is_one_dim ? signal_shape[0] : signal_shape[0] * signal_shape[1]) *
      sizeof(DataType);
  const auto divisor =
      is_one_dim ? std::pow(2, sub_number) : std::pow(4, sub_number / 3);
  return static_cast<size_t>(static_cast<double>(max_size) / divisor * 1.5);
}
}  // namespace drift
