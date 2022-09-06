import os

from conans import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout
from conans.tools import Git

required_conan_version = ">=1.50"


class WaveletBufferConan(ConanFile):
    name = "wavelet_buffer"
    version = "0.4.0"
    license = "MPL-2.0"
    author = "PANDA GmbH"
    description = "An universal C++ compression library based on wavelet transformation"
    url = "https://github.com/panda-official/WaveletBuffer"
    requires = "openblas/0.3.20", "blaze/3.8", "libjpeg-turbo/2.1.2", "cimg/3.0.2"
    default_options = {
        "cimg:enable_fftw": False,
        "cimg:enable_jpeg": False,
        "cimg:enable_openexr": False,
        "cimg:enable_png": False,
        "cimg:enable_tiff": False,
        "cimg:enable_ffmpeg": False,
        "cimg:enable_opencv": False,
        "shared": False,
        "fPIC": True,
    }

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}

    generators = "CMakeDeps"

    def set_version(self):
        suffix = os.getenv("VERSION_SUFFIX")
        if suffix:
            self.version += f"-b.{suffix}"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

        self.settings.build_type = "Release"

    def source(self):
        local_source = os.getenv("CONAN_SOURCE_DIR")
        if local_source is not None:
            print(f"Use local sources: {local_source}")
            self.run(
                "cp -r {}/. {}/".format(
                    os.getenv("CONAN_SOURCE_DIR"), self.source_folder
                )
            )
        else:
            branch = f"v{self.version}" if self.channel == "stable" else self.channel
            print(f"Use remote sources, branch: {branch}")
            git = Git()
            git.clone(
                "https://github.com/panda-official/WaveletBuffer.git",
                branch=branch,
                shallow=True,
            )

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["CONAN_EXPORTED"] = True
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["wavelet_buffer", "sf_compressor"]
