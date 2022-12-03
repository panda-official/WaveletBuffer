// Copyright 2021 PANDA GmbH

#include <wavelet_buffer/wavelet_buffer.h>

#include <fstream>
#include <sstream>

int main(int argc, char* argv[]) {
  /* Read file to std::string */
  std::ifstream file(argv[1]);
  std::stringstream blob;
  blob << file.rdbuf();

  auto wb = drift::WaveletBuffer::Parse(blob.str());

  return 0;
}
