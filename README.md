![example workflow](https://github.com/panda-official/WaveletBuffer/actions/workflows/ci.yml/badge.svg)
[![codecov](https://codecov.io/gh/panda-official/WaveletBuffer/branch/develop/graph/badge.svg?token=UWZLNR1PL6)](https://codecov.io/gh/panda-official/WaveletBuffer)

# WaveletBuffer
A universal  C++ compression library based on wavelet transformation

## Features

- Written in Modern C++
- One-side wavelet decomposition for vectors and matrixes
- 5 Daubechies Wavelets DB1-DB5
- Different denoising algorithms
- Fast and efficient compression with [SFCompressor](https://github.com/panda-official/SfCompressor)
- Cross-platform

## Requirements

* CMake >= 3.16
* C++20 compiler
* conan >= 1.53

## Bindings

* [Python](python/README.md)


## Usage Example
```c++
#include <wavelet_buffer/wavelet_buffer.h>

using drift::Signal1D;
using drift::WaveletBuffer;
using drift::WaveletParameters;
using drift::WaveletTypes;
using DenoiseAlgo = drift::ThresholdAbsDenoiseAlgorithm<float>;

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

## Build and Installing

On Ubuntu:

```
git clone https://github.com/panda-official/WaveletBuffer.git

mkdir build && cd build
cmake -DWB_BUILD_TESTS=ON -DWB_BUILD_BENCHMARKS=ON -DWB_BUILD_EXAMPLES=ON -DCODE_COVERAGE=ON ..
cmake --build . --target install
```

On MacOS:

```
git clone https://github.com/panda-official/WaveletBuffer.git
mkdir build && cd build
cmake -DWB_BUILD_TESTS=ON -DWB_BUILD_BENCHMARKS=ON -DWB_BUILD_EXAMPLES=ON -DCODE_COVERAGE=ON ..
cmake --build . --target install
```

On Windows:

```
git clone https://github.com/panda-official/WaveletBuffer.git
mkdir build && cd build
cmake -DWB_BUILD_TESTS=ON -DWB_BUILD_BENCHMARKS=ON -DWB_BUILD_EXAMPLES=ON -DCODE_COVERAGE=ON ..
cmake --build . --config Release --target install
```

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

## References

* [Documentation](https://waveletbuffer.readthedocs.io)
* [Drift Protocol](https://github.com/panda-official/DriftProtocol) - Protobuf Libraries to encode message in Drift infrastructure
* [Drift Python Client](https://github.com/panda-official/DriftPythonClient) - Python Client to access data of _PANDA|Drift_
