cmake_minimum_required(VERSION 3.16)
project(wavelet_buffer_py DESCRIPTION "wavelet_buffer python binding")

# Include wavelet_buffer
add_subdirectory(../ ../)

if(SKBUILD)
    # Scikit-Build does not add your site-packages to the search path
    # automatically, so we need to add it _or_ the pybind11 specific directory
    # here.
    execute_process(
        COMMAND
            "${PYTHON_EXECUTABLE}" -c
            "import pybind11; print(pybind11.get_cmake_dir())"
        OUTPUT_VARIABLE _tmp_dir
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ECHO STDOUT
    )
    list(APPEND CMAKE_PREFIX_PATH "${_tmp_dir}")
endif()

find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(_wavelet_buffer MODULE
        src/main.cc
        src/wavelet_parameters.cc
        src/blaze_utils.cc
        src/wavelet_buffer.cc
        src/wavelet_buffer_view.cc
        src/denoise.cc
        src/img/codecs.cc
        src/img/wavelet_image.cc
)

target_link_libraries(_wavelet_buffer PRIVATE wavelet_buffer::wavelet_buffer)
target_compile_definitions(
    _wavelet_buffer
    PRIVATE PYBIND11_DETAILED_ERROR_MESSAGES
)
target_include_directories(_wavelet_buffer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/)

install(TARGETS _wavelet_buffer DESTINATION .)
