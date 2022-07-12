cmake_minimum_required(VERSION 3.16)

project(wavelet_buffer VERSION 0.1.1)

option(WB_BUILD_TESTS "Enable unit tests" OFF)
option(WB_BUILD_EXAMPLES "Enable examples" OFF)
option(WB_BUILD_BENCHMARKS "Enable benchmarks" OFF)

# Conan
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_BINARY_DIR})

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(
        STATUS
        "Downloading conan.cmake from https://github.com/conan-io/cmake-conan"
    )
    file(
        DOWNLOAD
            "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake"
        TLS_VERIFY ON
    )
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_configure(REQUIRES openblas/0.3.20
                                blaze/3.8
        GENERATORS cmake
)

conan_cmake_autodetect(settings)

conan_cmake_install(PATH_OR_REFERENCE .
        BUILD missing
        REMOTE conancenter
        SETTINGS ${settings}
)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# Dependencies
include(FetchContent)

# SFCompressor
fetchcontent_declare(
    SFCompressor
    URL
        https://github.com/panda-official/SfCompressor/archive/refs/tags/v1.0.1.tar.gz
)

fetchcontent_makeavailable(SFCompressor)

# Create wb target
set(WB_TARGET_NAME ${PROJECT_NAME})
add_library(
    ${WB_TARGET_NAME}
    sources/wavelet_buffer.cc
    sources/wavelet_utils.cc
    sources/wavelet_buffer_view.cc
    sources/padding.cc
    sources/wavelet.cc
)

# Add alias
add_library(${WB_TARGET_NAME}::${WB_TARGET_NAME} ALIAS ${WB_TARGET_NAME})

# Set fPIC
set_target_properties(${WB_TARGET_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(sf_compressor PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Turn off parallelization in blaze
target_compile_definitions(
    ${WB_TARGET_NAME}
    PUBLIC BLAZE_USE_SHARED_MEMORY_PARALLELIZATION=0
)

# Set compiler features
target_compile_features(${WB_TARGET_NAME} PUBLIC cxx_std_17)

# Include paths
target_include_directories(
    ${WB_TARGET_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Link dependencies
target_link_libraries(${WB_TARGET_NAME} sf_compressor)
target_link_libraries(${WB_TARGET_NAME} CONAN_PKG::blaze)
target_link_libraries(${WB_TARGET_NAME} CONAN_PKG::openblas)

# Catch2 installation
if(WB_BUILD_TESTS OR WB_BUILD_BENCHMARKS)
    fetchcontent_declare(
        Catch2
        URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.0.1.tar.gz
        URL_HASH
            SHA256=8c4173c68ae7da1b5b505194a0c2d6f1b2aef4ec1e3e7463bde451f26bbaf4e7
    )

    fetchcontent_makeavailable(Catch2)

    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

    enable_testing()
    include(CTest)
    include(Catch)
endif()

# Add tests
if(WB_BUILD_TESTS)
    message(STATUS "Unit tests enabled")
    add_subdirectory(tests)
endif()

# Add benchmarks
if(WB_BUILD_BENCHMARKS)
    message(STATUS "Benchmarks enabled")
    add_subdirectory(benchmarks)
endif()

# Add examples
if(WB_BUILD_EXAMPLES)
    message(STATUS "Examples enabled")
    add_subdirectory(examples)
endif()

# Install rules
include(GNUInstallDirs)

# Create package targets file
install(TARGETS ${WB_TARGET_NAME} EXPORT ${WB_TARGET_NAME}-target)
install(
    EXPORT ${WB_TARGET_NAME}-target
    FILE ${WB_TARGET_NAME}-targets.cmake
    NAMESPACE ${WB_TARGET_NAME}::
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake
)

include(CMakePackageConfigHelpers)

# Create package config file
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/${WB_TARGET_NAME}-config.cmake.in"
    "${WB_TARGET_NAME}-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake
)
# Create package version file
write_basic_package_version_file(
    ${WB_TARGET_NAME}-config-version.cmake
    COMPATIBILITY ExactVersion
)

# Install cmake files
install(
    FILES
        "${PROJECT_BINARY_DIR}/${WB_TARGET_NAME}-config.cmake"
        "${PROJECT_BINARY_DIR}/${WB_TARGET_NAME}-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake
)

# Install headers
install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/wavelet_buffer
    TYPE INCLUDE
    PATTERN "*.md"
    EXCLUDE
)
