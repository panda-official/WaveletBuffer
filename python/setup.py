from skbuild import setup  # This line replaces 'from setuptools import setup'
from pathlib import Path

this_directory = Path(__file__).parent
long_description = (this_directory / "README.md").read_text()

setup(
    name='wavelet-buffer',
    version='0.1.0',
    packages=['wavelet_buffer'],
    package_dir={"": "src"},
    cmake_install_dir='src/wavelet_buffer',
    extras_require={"test": ["pytest", "numpy"]},
    author="PANDA, GmbH",
    author_email="info@panda.technology",
    description="WaveletBuffer python3 module",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/panda-official/WaveletBuffer",
    python_requires='>=3.6',
    license='MPL-2.0',
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Science/Research",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: Mozilla Public License 2.0 (MPL 2.0)",
        "Operating System :: Unix",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3 :: Only",
        "Programming Language :: Python :: Implementation :: CPython",
        "Topic :: Software Development",
        "Topic :: Scientific/Engineering",
    ]
)
