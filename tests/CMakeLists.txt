add_executable(unit_tests wavelet_buffer_test.cc)
target_link_libraries(unit_tests PRIVATE ${WB_TARGET_NAME})
target_link_libraries(unit_tests PRIVATE Catch2::Catch2)

# Discover tests
catch_discover_tests(unit_tests)