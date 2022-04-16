cmake_minimum_required(VERSION 3.16)

project(wavelet_buffer)

option(WB_BUILD_TESTS "Enable unit tests" OFF)
option(EXAMPLES_ENABLED "Enable examples" OFF)
option(BENCHMARKS_ENABLED "Enable benchmarks" OFF)

# Dependencies
include(FetchContent)

# SFCompressor
FetchContent_Declare(
        SFCompressor
        URL https://github.com/panda-official/SfCompressor/archive/refs/tags/v1.0.1.tar.gz
)

# Metric
FetchContent_Declare(
        Metric
        URL https://github.com/panda-official/metric/archive/refs/tags/v0.2.0.tar.gz
)

FetchContent_MakeAvailable(SFCompressor Metric)


set(WB_TARGET_NAME ${PROJECT_NAME})

# Create wb target
add_library(${WB_TARGET_NAME} sources/wavelet_buffer.cc
                                sources/wavelet_utils.cc
                                sources/wavelet_buffer_view.cc
                                sources/padding.cc)

# Set compiler features
target_compile_features(${WB_TARGET_NAME} PUBLIC cxx_std_17)

# Include paths
target_include_directories(${WB_TARGET_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(${WB_TARGET_NAME} sf_compressor)
target_link_libraries(${WB_TARGET_NAME} panda_metric)

# Catch2 installation
if (WB_BUILD_TESTS OR WB_BUILD_BENCHMARKS)
    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.0.0-preview4)

    FetchContent_MakeAvailable(Catch2)

    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

    enable_testing()
    include(CTest)
    include(Catch)
endif ()

# Add tests
if (WB_BUILD_TESTS)
    message(STATUS "Unit tests enabled")
    add_subdirectory(tests)
endif ()

