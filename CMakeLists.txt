cmake_minimum_required(VERSION 3.16)

project(wavelet_buffer VERSION 0.1.0)

option(WB_BUILD_TESTS "Enable unit tests" OFF)
option(WB_BUILD_EXAMPLES "Enable examples" OFF)
option(WB_BUILD_BENCHMARKS "Enable benchmarks" OFF)

#find_package(nlohmann_json REQUIRED)

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

# Add alias                              
add_library(${WB_TARGET_NAME}::${WB_TARGET_NAME} ALIAS ${WB_TARGET_NAME})

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

# Add benchmarks
if (WB_BUILD_BENCHMARKS)
  message(STATUS "Benchmarks enabled")
  add_subdirectory(benchmarks)
endif ()

# Add examples
if (WB_BUILD_EXAMPLES)
	message(STATUS "Examples enabled")
	add_subdirectory(examples)
endif ()


# Install rules
include(GNUInstallDirs)

# Create package targets file
install(TARGETS ${WB_TARGET_NAME} EXPORT ${WB_TARGET_NAME}-target)
install(EXPORT ${WB_TARGET_NAME}-target
        FILE ${WB_TARGET_NAME}-targets.cmake
        NAMESPACE ${WB_TARGET_NAME}::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

include(CMakePackageConfigHelpers)

# Create package config file
configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/${WB_TARGET_NAME}-config.cmake.in"
        "${WB_TARGET_NAME}-config.cmake"
        INSTALL_DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake
)
# Create package version file
write_basic_package_version_file(${WB_TARGET_NAME}-config-version.cmake COMPATIBILITY ExactVersion)

# Install cmake files
install(FILES "${PROJECT_BINARY_DIR}/${WB_TARGET_NAME}-config.cmake"
        "${PROJECT_BINARY_DIR}/${WB_TARGET_NAME}-config-version.cmake"
        DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

# Install headers
install(DIRECTORY ${PROJECT_SOURCE_DIR}/wavelet_buffer TYPE INCLUDE
        PATTERN "*.md" EXCLUDE)
