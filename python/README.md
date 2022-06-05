# Drift-py

A python wrapper for Drift Framework

# Build from the source

```
git clone --recurse-submodules git@gitlab.panda.technology:drift/sdk/drift_py.git
```

## Install Prerequisites

### Ubuntu

```
sudo apt-get install cmake build-essetial pkg-config

```

Install setuptools

```bash
pip install setuptools conan
```

## Build package

```
python setup.py bdist_wheel
```

### Install module

```
python -m pip install dist/*
```

## Examples

```python
from drift.dsp import WaveletImage, WaveletType, denoise, codecs, ColorSpace

params = dict(
    decomposition_steps=5, signal_shape=[10, 10], signal_number=1,
    wavelet_type=WaveletType.DB3
)

img = WaveletImage(**params)

img.import_from_file(
    'test.jpeg', denoise.Simple(compression_level=0.9),
    codecs.RgbJpeg())

img.save('output.bin')
```

for more examples please check `examples/` folder
