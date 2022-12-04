// Copyright 2021 PANDA GmbH

#include <wavelet_buffer/wavelet_buffer.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
  }

  std::cout << "Load " << argv[1] << std::endl;

  // drift::WaveletParameters params1, params2;
  // blaze::Archive<std::ifstream> archive(argv[1]);
  // archive >> params1 >> params2;

  // std::cout << params1 << std::endl;
  // std::cout << params2 << std::endl;

  // return 0;

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

  std::cout << "Loaded" << std::endl;

  return EXIT_SUCCESS;
}
