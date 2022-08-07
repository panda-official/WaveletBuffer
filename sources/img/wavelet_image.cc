// Copyright 2020-2022 PANDA GmbH

#include "wavelet_buffer/img/wavelet_image.h"

#include <fstream>
#include <utility>
#include <vector>

#include "wavelet_buffer/wavelet_buffer_view.h"
#include "wavelet_buffer/wavelet_utils.h"

namespace drift::img {

class WaveletImage::WaveletImageImpl {
 public:
  explicit WaveletImageImpl(WaveletParameters parameters)
      : buffer_(std::move(parameters)) {}

  explicit WaveletImageImpl(WaveletImage::Buffer buffer)
      : buffer_(std::move(buffer)) {}

  WaveletImage::Status ImportFromFile(const std::string &file_path,
                                      const Denoiser &denoiser,
                                      const Codec &codec,
                                      size_t start_channel) {
    Status status;

    std::ifstream file(file_path, std::ios::binary);
    if (file) {
      std::stringstream ss;
      ss << file.rdbuf();
      status = ImportFromString(ss.str(), denoiser, codec, start_channel);
    } else {
      status = Status::kIOError;
    }

    return status;
  }

  Status ExportToFile(const std::string &file_path, const Codec &codec,
                      size_t start_channel) const {
    Status status;

    std::ofstream file(file_path, std::ios::binary);
    if (file) {
      std::string data;
      status = ExportToString(&data, codec, start_channel);
      if (status == Status::kOk) {
        file << data;
      }
    } else {
      status = Status::kIOError;
    }

    return status;
  }

  WaveletImage::Status ImportFromString(const std::string &data,
                                        const Denoiser &denoiser,
                                        const Codec &codec,
                                        size_t start_channel) {
    Status status = Status::kOk;

    SignalN2D image;
    if (codec.Decode(data, &image)) {
      if (!buffer_(start_channel, image.size()).Decompose(image, denoiser)) {
        status = Status::kDecompositionError;
      }
    } else {
      status = Status::kWrongData;
    }

    return status;
  }

  WaveletImage::Status ExportToString(std::string *data, const Codec &codec,
                                      size_t start_channel) const {
    auto status = Status::kOk;
    SignalN2D image;

    if (buffer_(start_channel, codec.channel_number()).Compose(&image)) {
      if (!codec.Encode(image, data)) {
        status = Status::kWrongData;
      }
    } else {
      status = Status::kCompositionError;
    }

    return status;
  }

  Status Save(const std::string &file_path) const {
    Status status;
    std::ofstream file(file_path, std::ios::binary);
    if (file) {
      std::string data;
      status = buffer_.Serialize(&data) ? Status::kOk : Status::kWrongData;
      if (status == Status::kOk) {
        file << data;
      }
    } else {
      status = Status::kIOError;
    }

    return status;
  }

  Buffer buffer_;
};

WaveletImage::WaveletImage(size_t channel_number, size_t w, size_t h)
    : impl_(std::make_unique<WaveletImageImpl>(WaveletParameters{
          .signal_shape = {w, h},
          .signal_number = channel_number,
          .decomposition_steps = 0,
          .wavelet_type = WaveletTypes::kDB1,
      })) {}

WaveletImage::WaveletImage(WaveletParameters parameters)
    : impl_(std::make_unique<WaveletImageImpl>(std::move(parameters))) {}

WaveletImage::WaveletImage(WaveletImage::Buffer buffer)
    : impl_(std::make_unique<WaveletImageImpl>(std::move(buffer))) {}

WaveletImage::~WaveletImage() = default;

WaveletImage::Status WaveletImage::ImportFromFile(const std::string &file_path,
                                                  const Denoiser &denoiser,
                                                  const Codec &codec,
                                                  size_t start_channel) {
  return impl_->ImportFromFile(file_path, denoiser, codec, start_channel);
}

WaveletImage::Status WaveletImage::ExportToFile(const std::string &file_path,
                                                const Codec &codec,
                                                size_t start_channel) const {
  return impl_->ExportToFile(file_path, codec, start_channel);
}

WaveletImage::Status WaveletImage::ImportFromString(const std::string &data,
                                                    const Denoiser &denoiser,
                                                    const Codec &codec,
                                                    size_t start_channel) {
  return impl_->ImportFromString(data, denoiser, codec, start_channel);
}

WaveletImage::Status WaveletImage::ExportToString(std::string *data,
                                                  const Codec &codec,
                                                  size_t start_channel) const {
  return impl_->ExportToString(data, codec, start_channel);
}

std::unique_ptr<WaveletImage> WaveletImage::Load(const std::string &file_path) {
  std::unique_ptr<WaveletImage> img;

  std::ifstream file(file_path, std::ios::binary);
  if (file) {
    std::stringstream ss;
    ss << file.rdbuf();
    auto buffer = Buffer::Parse((ss.str()));
    if (buffer) {
      img = std::make_unique<WaveletImage>(std::move(*buffer));
    }
  }

  return img;
}

WaveletImage::Status WaveletImage::Save(const std::string &file_path) const {
  return impl_->Save(file_path);
}

const WaveletImage::Buffer &WaveletImage::const_buffer() const {
  return impl_->buffer_;
}

WaveletImage::Buffer &WaveletImage::buffer() { return impl_->buffer_; }

double WaveletImage::CompareTo(const WaveletImage &other) const {
  return Distance(const_buffer(), other.const_buffer());
}

}  // namespace drift::img
