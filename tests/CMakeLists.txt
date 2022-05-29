add_executable(unit_tests wavelet_buffer_test.cc
        denoise_algorithms_test.cc
        padding_test.cc
        wavelet_parameters_test.cc
        #wavelet_buffer_view_test.cc
        )
target_link_libraries(unit_tests PRIVATE ${WB_TARGET_NAME})
target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain)

# Discover tests
catch_discover_tests(unit_tests)