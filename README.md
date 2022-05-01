![example workflow](https://github.com/panda-official/WaveletBuffer/actions/workflows/test-linux.yml/badge.svg)

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