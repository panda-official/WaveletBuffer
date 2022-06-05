cmake_minimum_required(VERSION 3.16)
project(drift_py DESCRIPTION "drift python binding")


#if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
#    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
#    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.16.1/conan.cmake"
#            "${CMAKE_BINARY_DIR}/conan.cmake"
#            EXPECTED_HASH SHA256=396e16d0f5eabdc6a14afddbcfff62a54a7ee75c6da23f32f7a31bc85db23484
#            TLS_VERIFY ON)
#endif()
#
#include(${CMAKE_BINARY_DIR}/conan.cmake)
#
#conan_cmake_autodetect(settings)
#conan_cmake_install(PATH_OR_REFERENCE ${CMAKE_SOURCE_DIR}/conanfile.txt
#        BUILD missing
#        SETTINGS ${settings}
#        PROFILE ${CONAN_PROFILE})

#include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
#conan_basic_setup(NO_OUTPUT_DIRS ON)

#find_package(Python3 COMPONENTS Interpreter Development)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# --------------------------------------- base python module ----------------------------
#add_library(base_python_module INTERFACE)
#
#target_include_directories(base_python_module INTERFACE
#        ${SOURCE_PATH}
#        ${CMAKE_SOURCE_DIR}/src
#        ${CMAKE_BINARY_DIR})
#
#target_link_libraries(base_python_module INTERFACE
#        Python3::Module
#        ${CONAN_LIBS})

if(SKBUILD)
    # Scikit-Build does not add your site-packages to the search path
    # automatically, so we need to add it _or_ the pybind11 specific directory
    # here.
    execute_process(
            COMMAND "${PYTHON_EXECUTABLE}" -c
            "import pybind11; print(pybind11.get_cmake_dir())"
            OUTPUT_VARIABLE _tmp_dir
            OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ECHO STDOUT)
    list(APPEND CMAKE_PREFIX_PATH "${_tmp_dir}")
endif()

find_package(pybind11 CONFIG REQUIRED)

add_subdirectory(src)

install(TARGETS _wb DESTINATION .)
