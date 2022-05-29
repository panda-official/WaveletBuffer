add_executable(unit_tests wavelet_buffer_test.cc
        denoise_algorithms_test.cc
        padding_test.cc
        wavelet_parameters_test.cc
        #wavelet_buffer_view_test.cc
        )
target_link_libraries(unit_tests PRIVATE ${WB_TARGET_NAME})
target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain)

# Code coverage
option(CODE_COVERAGE "Enable coverage report (only for debug build type)" OFF)

if (CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        target_compile_options(unit_tests PRIVATE --coverage)
        target_link_options(unit_tests PRIVATE --coverage)
        message(STATUS "Turn on code coverage")
    else (CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(WARNING "Code coverage only works with Debug build type. Skipping")
    endif (CMAKE_BUILD_TYPE STREQUAL Debug)
endif (CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

# Discover tests
catch_discover_tests(unit_tests)
