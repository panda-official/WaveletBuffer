cmake_minimum_required(VERSION 3.16)
project(wavelet_buffer_py DESCRIPTION "wavelet_buffer python binding")

if(USE_CONAN)
    #    list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
    #    list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
    #
    #    if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    #        message(
    #            STATUS
    #            "Downloading conan.cmake from https://github.com/conan-io/cmake-conan"
    #        )
    #        file(
    #            DOWNLOAD
    #                "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
    #                "${CMAKE_BINARY_DIR}/conan.cmake"
    #            TLS_VERIFY ON
    #        )
    #    endif()
    #
    #    include(${CMAKE_BINARY_DIR}/conan.cmake)
    #
    #    conan_cmake_configure(REQUIRES wavelet_buffer/[>=0.1.0]@drift/develop
    #            GENERATORS cmake_find_package
    #    )
    #
    #    conan_cmake_autodetect(settings)
    #
    #    conan_cmake_install(PATH_OR_REFERENCE .
    #            BUILD missing
    #            REMOTE conancenter
    #            SETTINGS ${settings}
    #    )
    #
    find_package(wavelet_buffer)
elseif()
    add_subdirectory(../ ../)
endif()

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
        src/init.cc
        src/enums.cc
        src/blaze_utils.cc
        src/wavelet_buffer.cc
        src/wavelet_buffer_view.cc
        src/denoise.cc
)

target_link_libraries(_wavelet_buffer PRIVATE wavelet_buffer::wavelet_buffer)

install(TARGETS _wavelet_buffer DESTINATION .)
