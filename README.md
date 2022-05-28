![example workflow](https://github.com/panda-official/WaveletBuffer/actions/workflows/test-linux.yml/badge.svg)
[![codecov](https://codecov.io/gh/panda-official/WaveletBuffer/branch/develop/graph/badge.svg?token=UWZLNR1PL6)](https://codecov.io/gh/panda-official/WaveletBuffer)

# WaveletBuffer
An universal  C++ compression library based on wavelet transformation

## Requirements
* CMake >= 3.16
* LAPACK
* blaze 3.8 https://bitbucket.org/blaze-lib/blaze/wiki/Home
* nlohmann_json https://github.com/nlohmann/json
* cereal https://github.com/USCiLab/cereal

## Build and install
* Download and unpack [release](https://github.com/panda-official/WaveletBuffer/releases)
* `cd <wavelet_buffer_dir>`
* `mkdir build && cd build`
* `cmake --build .. // -DWB_{TESTS,EXAMPLES,BENCHMARKS}=ON for tests, examples and benchmarks`
* `cmake --install ..`

## Integration
### Using cmake target
```cmake
find_package(wavelet_buffer REQUIRED)

add_executable(program program.cpp)
target_link_libraries(program wavelet_buffer::wavelet_buffer)

# WaveletBuffer use blaze as linear algebra library which expects you to have a LAPACK library installed
# (it will still work without LAPACK and will not be reduced in functionality, but performance may be limited)
find_package(LAPACK REQUIRED)
target_link_libraries(program ${LAPACK_LIBRARIES})
```

## Example
```c++
#include <wavelet_buffer/wavelet_buffer.h>

using drift::dsp::Signal1D;
using drift::dsp::WaveletBuffer;
using drift::dsp::WaveletParameters;
using drift::dsp::WaveletTypes;
using DenoiseAlgo = drift::dsp::ThresholdAbsDenoiseAlgorithm<float>;

int main() {
  Signal1D original = blaze::generate(
      1000, [](auto index) { return static_cast<float>(index % 100); });

  std::cout << "Original size: " << original.size() * 4 << std::endl;
  WaveletBuffer buffer(WaveletParameters{
      .signal_shape = {original.size()},
      .signal_number = 1,
      .decomposition_steps = 3,
      .wavelet_type = WaveletTypes::kDB1,
  });

  // Wavelet decomposition of the signal and denoising 
  buffer.Decompose(original, DenoiseAlgo(0, 0.3));

  // Compress the buffer
  std::string arch;
  buffer.Serialize(&arch, 16);
  std::cout << "Compressed size: " << arch.size() << std::endl;

  // Decompress the buffer
  auto restored_buffer = WaveletBuffer::Parse(arch);
  Signal1D output_signal;

  // Restore the signal from wavelet decomposition
  restored_buffer->Compose(&output_signal);

  std::cout << "Distance between original and restored signal: "
            << blaze::norm(original - output_signal) / original.size()
            << std::endl;
  std::cout << "Compression rate: " << original.size() * 4. / arch.size() * 100
            << "%" << std::endl;
}
```
