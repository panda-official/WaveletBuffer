# wavelet-buffer

A python wrapper for WaveletBuffer Framework

# Install from PyPI
`pip install wavelet-buffer`

# Install from repo
## Dependencies (Ubuntu)
```bash
sudo apt-get install libopenblas-dev
sudo apt-get install libjpeg-dev
sudo apt-get install nlohmann-json3-dev
git clone https://bitbucket.org/blaze-lib/blaze.git --depth=1 --branch=v3.8 && cd blaze && cmake -DCMAKE_INSTALL_PREFIX=/usr/local/ . && sudo make install
git clone https://github.com/USCiLab/cereal --depth=1 --branch=v1.3.2 && cd cereal && cmake -DCMAKE_INSTALL_PREFIX=/usr/local/ -DJUST_INSTALL_CEREAL=ON . && sudo make install
````
# Build and install
`pip install git+https://github.com/panda-official/WaveletBuffer.git`

## Examples

```python
import numpy as np
from wavelet_buffer import WaveletBuffer, WaveletType, denoise

signal = np.sin(np.linspace(0, np.pi, 100) * 10)
print(f"Original signal: {signal[0:10]}")

buffer = WaveletBuffer(signal_shape=signal.shape, signal_number=1,
                       decomposition_steps=2, wavelet_type=WaveletType.DB3)

# Denoise and serialize data
buffer.decompose(signal, denoise.Threshold(0, 0.05))
arch = buffer.serialize(compression_level=16)

print(f"Size of archive: {len(arch)} bytes")

# Restore data from archive
buffer: WaveletBuffer = WaveletBuffer.parse(arch)
restored_signal = buffer.compose()
print(f"Restored signal: {restored_signal[0:10]}")
```

for more examples please check `examples/` folder
