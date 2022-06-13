from skbuild import setup  # This line replaces 'from setuptools import setup'
from setuptools import find_packages

setup(
    name='wavelet-buffer',
    packages=['wavelet_buffer'],
    package_dir={"": "src"},
    cmake_install_dir='src/wavelet_buffer'
)
