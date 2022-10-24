"""Utility functions for WaveletBuffer"""
import wavelet_buffer._wavelet_buffer as impl

from wavelet_buffer.wavelet_buffer import WaveletBuffer


def distance(a: WaveletBuffer, b: WaveletBuffer) -> float:
    """Euclidian instance between two wavelet buffer

    The method calculates the distance without reconstruction the original signal
    Returns:
        normalized distance
    """
    return impl.distance(a, b)


def energy_distribution(buffer: WaveletBuffer) -> "np.ndarray[np.float32]":
    """Calculate energy for each subbands
    Args:
        buffer: input buffer for calculation
    Returns:
        energy distribution as a array with layout: [D1, D2, D3, .. Dn, An]

    """
    return impl.energy_distribution(buffer)
