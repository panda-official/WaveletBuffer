cmake_minimum_required(VERSION 3.16)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.24")
    cmake_policy(SET CMP0135 NEW)
endif()

project(wavelet_buffer VERSION 0.7.0)

option(WB_BUILD_TESTS "Enable unit tests" OFF)
option(WB_BUILD_EXAMPLES "Enable examples" OFF)
option(WB_BUILD_BENCHMARKS "Enable benchmarks" OFF)

# Conan
if(CONAN_EXPORTED)
    message(STATUS "Conan exported")
else()
    message(STATUS "Run conan")

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

    conan_cmake_configure(REQUIRES
            blaze/3.8
            libjpeg-turbo/2.1.3
            cimg/3.0.2
            catch2/3.2.1
            OPTIONS
            cimg:enable_fftw=False
            cimg:enable_jpeg=False # To avoid conflict with libjpeg-turbo
            cimg:enable_openexr=False
            cimg:enable_png=False
            cimg:enable_tiff=False
            cimg:enable_ffmpeg=False
            cimg:enable_opencv=False
            GENERATORS CMakeDeps
    )
    conan_cmake_autodetect(settings)
    conan_cmake_install(PATH_OR_REFERENCE .
            BUILD missing
            REMOTE conancenter
            SETTINGS ${settings}
    )
endif()

find_package(blaze REQUIRED)
find_package(libjpeg-turbo REQUIRED)
find_package(cimg REQUIRED)
find_package(Catch2 REQUIRED)

# Create wb target
set(WB_TARGET_NAME ${PROJECT_NAME})
add_library(
    ${WB_TARGET_NAME}
    sources/wavelet_buffer.cc
    sources/wavelet_buffer_serializer.cc
    sources/wavelet_utils.cc
    sources/wavelet_buffer_view.cc
    sources/padding.cc
    sources/wavelet.cc
    sources/img/wavelet_image.cc
    sources/img/color_space.cc
    sources/img/jpeg_codecs.cc
    # Internal
    sources/internal/sf_compressor.cc
    sources/internal/matrix_compressor.cc
)

include(FetchContent)

fetchcontent_declare(
    streamvbyte
    GIT_REPOSITORY https://github.com/lemire/streamvbyte.git
    GIT_TAG 7da04c7df63681a47f34a47ab457d6527825adbc
)

fetchcontent_declare(
    fpzip
    GIT_REPOSITORY https://github.com/victor1234/fpzip.git
    GIT_TAG drift-637
)

fetchcontent_makeavailable(streamvbyte)
fetchcontent_makeavailable(fpzip)

# Add alias
add_library(${WB_TARGET_NAME}::${WB_TARGET_NAME} ALIAS ${WB_TARGET_NAME})

# Set fPIC
set_target_properties(${WB_TARGET_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(streamvbyte PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(fpzip PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Turn off parallelization in blaze
target_compile_definitions(
    ${WB_TARGET_NAME}
    PUBLIC BLAZE_USE_SHARED_MEMORY_PARALLELIZATION=0
)

# Set CImg options
target_compile_definitions(cimg::cimg INTERFACE cimg_display=0)

# Set compiler features
target_compile_features(${WB_TARGET_NAME} PUBLIC cxx_std_20)
set_target_properties(${WB_TARGET_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)

# target_compile_options(${WB_TARGET_NAME} PUBLIC -D_GLIBCXX_DEBUG)
# target_compile_options(sf_compressor PUBLIC -D_GLIBCXX_DEBUG)

# Include paths
target_include_directories(
    ${WB_TARGET_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_include_directories(
    ${WB_TARGET_NAME}
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/sources
)

# Link dependencies
target_link_libraries(${WB_TARGET_NAME} PRIVATE streamvbyte)
target_link_libraries(${WB_TARGET_NAME} PRIVATE fpzip)
target_link_libraries(${WB_TARGET_NAME} PRIVATE libjpeg-turbo::libjpeg-turbo)
target_link_libraries(${WB_TARGET_NAME} PRIVATE cimg::cimg)

target_link_libraries(${WB_TARGET_NAME} PUBLIC blaze::blaze)

# Catch2 installation
if(WB_BUILD_TESTS OR WB_BUILD_BENCHMARKS)
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
install(TARGETS ${WB_TARGET_NAME} streamvbyte EXPORT ${WB_TARGET_NAME}-target)
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
