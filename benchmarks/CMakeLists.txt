add_executable(
    benchmarks
    denoise_algorithms_benchmark.cc
    wavelet_buffer_benchmarks.cc
    wavelet_buffer_util_benchmarks.cc
    img/jpeg_codec_benchmarks.cc
    init.cc
)

target_link_libraries(benchmarks PRIVATE ${WB_TARGET_NAME})
target_link_libraries(benchmarks PRIVATE Catch2::Catch2WithMain)

# Copy fixtures
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/img/fixtures/pic100x100.jpg
    ${CMAKE_CURRENT_BINARY_DIR}/img/fixtures/pic100x100.jpg
    COPYONLY
)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/img/fixtures/pic200x200.jpg
    ${CMAKE_CURRENT_BINARY_DIR}/img/fixtures/pic200x200.jpg
    COPYONLY
)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/img/fixtures/pic400x400.jpg
    ${CMAKE_CURRENT_BINARY_DIR}/img/fixtures/pic400x400.jpg
    COPYONLY
)
