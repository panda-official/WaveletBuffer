"""
Module test denoisers
"""
from wavelet_buffer import denoise  # pylint: disable=no-name-in-module


class TestNullDenoiser:  # pylint: disable=too-few-public-methods
    """
    Test Null Denoiser
    """

    def test__construct(self):  # pylint: disable=no-self-use
        """
        Test constructor
        """
        denoise.Null()


class TestSimpleDenoiser:  # pylint: disable=too-few-public-methods
    """
    Test Simple Denoiser
    """

    def test__construct(self):  # pylint: disable=no-self-use
        """
        Test constructor
        """
        denoise.Simple(compression_level=1)
        denoise.Simple(compression_level=2)
        denoise.Simple(compression_level=0)
        denoise.Simple(compression_level=0.5)


class TestThresholdDenoiser:  # pylint: disable=too-few-public-methods
    """
    Test Threshold Denoiser
    """

    def test__construct(self):  # pylint: disable=no-self-use
        """
        Test constructor
        """
        denoise.Threshold(a=0, b=-1)
        denoise.Threshold(a=-6.7, b=0)
        denoise.Threshold(a=22, b=67.2)
        denoise.Threshold(a=-3.78, b=1.5)
