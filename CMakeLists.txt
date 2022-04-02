cmake_minimum_required(VERSION 3.16)

project(wavelet_buffer)

option(WB_BUILD_TESTS "Enable unit tests" OFF)
option(EXAMPLES_ENABLED "Enable examples" OFF)
option(BENCHMARKS_ENABLED "Enable benchmarks" OFF)

# Dependencies
include(FetchContent)

set(WB_TARGET_NAME ${PROJECT_NAME})

# Create wb target
add_library(${WB_TARGET_NAME} sources/wavelet_buffer.cc)

# Set compiler features
target_compile_features(${WB_TARGET_NAME} PUBLIC cxx_std_17)

# Include paths
target_include_directories(${WB_TARGET_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Catch2 installation
if (WB_BUILD_TESTS OR WB_BUILD_BENCHMARKS)
    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v2.13.7)

    FetchContent_MakeAvailable(Catch2)

    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)

    enable_testing()
    include(CTest)
    include(Catch)
endif ()

# Add tests
if (WB_BUILD_TESTS)
    message(STATUS "Unit tests enabled")
    add_subdirectory(tests)
endif ()

