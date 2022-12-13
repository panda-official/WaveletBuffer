// Copyright 2021 PANDA GmbH

#include <wavelet_buffer/wavelet_buffer.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Load " << argv[1] << std::endl;

  /* Read file to std::string */
  std::ifstream file(argv[1]);
  if (!file.is_open()) {
    std::cerr << "Could not open file " << argv[1] << std::endl;
    return EXIT_FAILURE;
  }

  std::stringstream blob;
  blob << file.rdbuf();

  /* Load wavelet buffer from binary */
  auto wb = drift::WaveletBuffer::Parse(blob.str());
  if (!wb) {
    std::cerr << "Could not parse wavelet buffer from file " << argv[1]
              << std::endl;
    return EXIT_FAILURE;
  } else {
    std::cout << "Loaded wavelet buffer with parameters:" << wb->parameters()
              << std::endl;
  }

  return EXIT_SUCCESS;
}
