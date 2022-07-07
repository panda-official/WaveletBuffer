import numpy as np

from wavelet_buffer import (
    WaveletBuffer,
    WaveletType,
    denoise,
    energy_distribution,
)

# Processing 1-D time series
signal = np.array([1, 2, 3, 3, 2, 1, 0, -1, -2, -3, -3, -2, -1, 0, 1, 2, 3, 3, 2, 1])

buffer = WaveletBuffer(
    signal_shape=[20],
    signal_number=1,
    decomposition_steps=2,
    wavelet_type=WaveletType.DB2,
)

print(f"Decompose and denoise signal: {signal}")

denoiser = denoise.Simple(0.7)  # just make 70% of the smallest values zeros
buffer.decompose(signal, denoiser)

print(energy_distribution(buffer))

restored_signal = buffer.compose()
print(f"Reconstructed signal {restored_signal}")

restored_signal = buffer.compose(scale_factor=1)
print(f"Reconstructed signal x2 smaller: {restored_signal}")

restored_signal = buffer.compose(scale_factor=2)
print(f"Reconstructed signal x4 smaller: {restored_signal}")

# Processing N2D matrix
buffer = WaveletBuffer(
    signal_shape=[800, 500],
    signal_number=3,
    decomposition_steps=4,
    wavelet_type=WaveletType.DB1,
)
