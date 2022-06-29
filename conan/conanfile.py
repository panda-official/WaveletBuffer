import os

from conans import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout
from conans.tools import Git


class WaveletBufferConan(ConanFile):
    name = "wavelet_buffer"
    version = "0.1.0"
    license = "MPL-2.0"
    author = "PANDA GmbH"
    description = "An universal C++ compression library based on wavelet transformation"
    url = "https://github.com/panda-official/WaveletBuffer"
    requires = "openblas/0.3.20", "blaze/3.8"

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    generators = "CMakeDeps"

    def set_version(self):
        if os.getenv("CI"):
            ref = os.getenv("GITHUB_REF")
            # If run on CI without git tag specification add workflow run id as
            # build postfix
            if ref.startswith("ref/heads/"):
                self.version += "-b." + os.getenv("GITHUB_RUN_ID")
            elif ref.startswith("ref/tags"):
                assert (
                    ref.split("/")[-1] == f"v{self.version}",
                    "Version from git tag doesn't match version from " "conanfile",
                )

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def source(self):
        local_source = os.getenv("CONAN_SOURCE_DIR")
        if local_source is not None:
            self.run(
                "cp {} -r {}".format(os.getenv("CONAN_SOURCE_DIR"), self.source_folder)
            )
        else:
            branch = f"v{self.version}" if self.channel == "stable" else self.channel
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
