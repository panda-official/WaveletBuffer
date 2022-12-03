// Copyright 2021 PANDA GmbH

#include <wavelet_buffer/wavelet_buffer.h>

#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char* argv[]) {
  /* Read file to std::string */
  std::cout << argv[1] << std::endl;
  std::ifstream file(argv[1]);
  std::stringstream blob;
  blob << file.rdbuf();

  /* Load wavelet buffer from  binary */
  auto wb = drift::WaveletBuffer::Parse(blob.str());

  return 0;
}
