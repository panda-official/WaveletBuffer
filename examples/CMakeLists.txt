add_executable(wavelet_buffer_example wavelet_buffer_example.cc)
target_link_libraries(wavelet_buffer_example PRIVATE ${WB_TARGET_NAME})
add_compile_definitions(
    wavelet_buffer_example
    INPUT_SIGNAL_PATH="${CMAKE_CURRENT_SOURCE_DIR}/input_signal.csv"
)
