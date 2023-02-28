
// Copyright 2020-2021 PANDA GmbH

#include <wavelet_buffer/img/jpeg_codecs.h>
#include <wavelet_buffer/img/wavelet_image.h>

#undef NDEBUG
#include <cassert>

using drift::NullDenoiseAlgorithm;
using drift::SignalN2D;
using drift::ThresholdAbsDenoiseAlgorithm;
using drift::WaveletParameters;
using drift::WaveletTypes;
using drift::img::GrayJpegCodec;
using drift::img::RgbJpegCodec;
using drift::img::WaveletImage;

int main() {
  const auto kImagePath = "../pandas.jpg";
  const auto kOutputImagePath = "pandas_out.jpg";
  const auto kWaveletParams = WaveletParameters{
      .signal_shape = {800, 500},
      .signal_number = 3,
      .decomposition_steps = 5,
      .wavelet_type = WaveletTypes::kDB3,
  };

  RgbJpegCodec codec;

  WaveletImage image(kWaveletParams);

  std::cout << "Importing image from " << kImagePath << " ...";
  auto status = image.ImportFromFile(kImagePath,
                                     ThresholdAbsDenoiseAlgorithm<float>(0, 0.07), codec);

  assert(status == WaveletImage::Status::kOk);
  std::cout << "OK" << std::endl;


  std::string blob;
  auto ret = image.const_buffer().Serialize(&blob, 20);
  assert(ret);

  std::cout << "Archive size = " << blob.size() / 1024 << "kB" << std::endl;


  WaveletImage restored(*drift::WaveletBuffer::Parse(blob));

  std::cout << "Exporting image to " << kOutputImagePath << " ...";
  status = restored.ExportToFile(kOutputImagePath, codec);

  assert(status == WaveletImage::Status::kOk);

  std::cout << "OK" << std::endl;
}
