from skbuild import setup  # This line replaces 'from setuptools import setup'
from setuptools import find_packages

setup(
    name='drift_py',
    packages=find_packages(where="src"),
    package_dir={"": "src"},
)