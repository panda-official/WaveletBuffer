// Copyright 2020-2021 PANDA GmbH

#include <wavelet_buffer/wavelet_buffer.h>
#include <wavelet_buffer/wavelet_utils.h>

#include <fstream>

using drift::Signal1D;
using drift::WaveletBuffer;
using drift::WaveletParameters;
using drift::WaveletTypes;
using DenoiseAlgo = drift::SimpleDenoiseAlgorithm<float>;

std::vector<float> ReadData() {
  std::ifstream cvs_file(INPUT_SIGNAL_PATH);
  std::array<char, 2048> row{};
  std::vector<float> data;
  if (cvs_file.getline(row.data(), row.size())) {
    int last = 0;
    for (int i = 0; row[i] != '\0' || i < row.size(); ++i) {
      if (row[i] == ',') {
        data.push_back(
            std::stof(std::string(row.begin() + last, row.begin() + i)));
        last = i + 1;
      }
    }

    data.push_back(std::stof(std::string(row.begin() + last, row.end())));
  } else {
    // LOG_ERROR() << "No signal read";
    return {};
  }

  return data;
}

int main() {
  auto data = ReadData();

  Signal1D signal(data.size(), data.data());
  WaveletBuffer buffer(WaveletParameters{
      .signal_shape = {signal.size()},
      .signal_number = 1,
      .decomposition_steps = 2,
      .wavelet_type = WaveletTypes::kDB3,
  });

  buffer.Decompose(signal, DenoiseAlgo(0.3));

  Signal1D output_signal;

  buffer.Compose(&output_signal, 2);
  // LOG_INFO() << "Approximation 2" << output_signal;

  buffer.Compose(&output_signal, 1);
  // LOG_INFO() << "Approximation 1" << output_signal;

  buffer.Compose(&output_signal);
  // LOG_INFO() << "Composed signal: " << output_signal;
}
