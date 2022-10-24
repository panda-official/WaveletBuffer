"""Collection of denoise algorithms
"""
import wavelet_buffer._wavelet_buffer.denoise as impl


class BaseDenoiser(impl.Base):
    pass


class Null(impl.Null):
    """Denoiser doesn't do anything"""

    pass


class Simple(impl.Simple):
    """Set to 0 a part of the smallest values in high frequency subbands"""

    def __init__(self, compression_level: float):
        """
        Args:
            compression_level: from 0 to 1. 0 is not compression, 1 all values in input are zeros.
        """
        super().__init__(compression_level)


class Threshold(impl.Threshold):
    """Set to zero values which absolute value less than threshold
    computed by `A * step + b`
    """

    def __init__(self, a, b):
        """
        Args
            a:
            b:
        """
        super().__init__(a, b)
