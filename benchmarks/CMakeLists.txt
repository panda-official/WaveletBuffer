add_executable(
    benchmarks
    denoise_algorithms_benchmark.cc
    wavelet_buffer_benchmarks.cc
    wavelet_buffer_util_benchmarks.cc
    init.cc
)

target_link_libraries(benchmarks PRIVATE ${WB_TARGET_NAME})
target_link_libraries(benchmarks PRIVATE Catch2::Catch2WithMain)
