import os
import shutil
from string import Template
from setuptools.command.sdist import sdist

try:
    from skbuild import setup  # This line replaces 'from setuptools import setup'
except ModuleNotFoundError:
    # use classical setuptools to build sdist
    from setuptools import setup

from pathlib import Path

HERE = Path(__file__).parent
DESC = (HERE / "README.md").read_text()

MAJOR_VERSION = 0
MINOR_VERSION = 1
PATCH_VERSION = 0

PACKAGE_NAME = "wavelet-buffer"

VERSION_SUFFIX = os.getenv("VERSION_SUFFIX")


def get_file_list(path):
    return path, [f"{folder}/{el}" for folder, _, elements in os.walk(path) for el in elements]


class BuildPyCommand(sdist):
    """Custom build command."""

    def run(self):
        self.copy_wavelet_buffer_source()
        sdist.run(self)

    @staticmethod
    def copy_wavelet_buffer_source():
        """We copy WaveletBuffer source when we build sdist because it is in the top folder"""
        project_root = HERE.parent.resolve().parent.resolve()
        map = [("", [project_root / "CMakeLists.txt"]),
               ("sources", [project_root / "sources"]),
               ("wavelet_buffer", [project_root / "wavelet_buffer"]),
               ("cmake", [project_root / "cmake"])]

        dest = HERE / "ext"
        if dest.exists():
            shutil.rmtree(dest)

        Path.mkdir(dest)
        for folder, files in map:
            print(folder)
            new_path = dest / folder
            for src in files:
                print(f"copy {src} -> {new_path}")
                if Path.is_file(src):
                    shutil.copy(src, new_path)
                else:
                    shutil.copytree(src, new_path)


def update_package_version(path: Path, version: str, protoc_version: str):
    """Overwrite/create __init__.py file and fill __version__"""
    template = (path / "__init__.py.in").read_text(encoding="utf-8")
    init_content = Template(template).substitute(
        version=version, protoc_version=protoc_version
    )
    with open(path / "__init__.py", "w") as f:
        f.write(init_content)


def build_version():
    """Build dynamic version and update version in package"""
    version = f"{MAJOR_VERSION}.{MINOR_VERSION}.{PATCH_VERSION}"
    if VERSION_SUFFIX:
        version += f"dev.{VERSION_SUFFIX}"

    return version


setup(
    name=PACKAGE_NAME,
    cmdclass={
        "sdist": BuildPyCommand
    },
    version=build_version(),
    packages=["wavelet_buffer"],
    package_dir={"": "pkg"},
    data_files=[get_file_list("src"), get_file_list("ext"),
                ("", ["CMakeLists.txt"]), ],

    extras_require={"test": ["pytest", "numpy"]},
    author="PANDA, GmbH",
    author_email="info@panda.technology",
    description="A universal C++ compression library based on wavelet transformation",
    long_description=DESC,
    long_description_content_type="text/markdown",
    url="https://github.com/panda-official/WaveletBuffer",
    python_requires=">=3.8",
    license="MPL-2.0",
    classifiers=[
        "Development Status :: 2 - Beta",
        "Intended Audience :: Science/Research",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: Mozilla Public License 2.0 (MPL 2.0)",
        "Operating System :: Unix",
        "Operating System :: MacOS"
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3 :: Only",
        "Programming Language :: Python :: Implementation :: CPython",
        "Topic :: Software Development",
        "Topic :: Scientific/Engineering",
    ],
)
