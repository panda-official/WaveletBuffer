""""Wavelet Buffer"""
from typing import List, Optional
import wavelet_buffer._wavelet_buffer as impl

from wavelet_buffer.denoise import BaseDenoiser


class WaveletType(impl.WaveletType):
    """Wavelet Type"""

    pass


class WaveletParameters(impl.WaveletParameters):
    """Parameters of wavelet decomposition"""

    signal_shape: List[int]
    """For 1D signal with size N [N], for 2DN signal with size MXN, [M,N]
    """

    signal_number: int
    """ N for 2DN signal, e.g. RGB image would be 3 signals
    """

    decomposition_steps: int
    """Number of decomposition steps
    """

    wavelet_type: WaveletType
    """Type WaveletType.NONE, DB{1..5}, if it is NONE no wavelet composition
    """


class WaveletBuffer(impl.WaveletBuffer):
    """Universal buffer for the wavelet decomposition"""

    def __init__(
        self,
        signal_shape: List[int],
        signal_number: int,
        decomposition_steps: int,
        wavelet_type: WaveletType,
        decompositions=None,
    ):
        """
        Args:
            signal_shape: for 1D signal with size N [N], for 2DN signal with size MXN, [M,N]
            signal_number: N for 2DN signal, e.g. RGB image would be 3 signals
            decomposition_steps: number of decomposition steps
            wavelet_type: type WaveletType.NONE, DB{1..5}, if it is NONE no wavelet composition
            decompositions: you can create a buffer with initialized subbands
        """
        if decompositions:
            super().__init__(
                signal_shape,
                signal_number,
                decomposition_steps,
                wavelet_type,
                decompositions,
            )
        else:
            super().__init__(
                signal_shape, signal_number, decomposition_steps, wavelet_type
            )

    @staticmethod
    def decomposition_size(
        signal_shape: List[int], decomposition_steps: int, **_kwargs
    ) -> int:
        """
        Estimate number of subbands for specific parameters
        Args:
            signal_shape: for 1D signal with size N [N], for 2DN signal with size MXN, [M,N]
            decomposition_steps: number of decomposition steps
        Returns:
            number of subbands for each signal
        """
        return impl.WaveletBuffer.decomposition_size(
            signal_shape, 0, decomposition_steps, WaveletType.NONE
        )

    @staticmethod
    def parse(blob: bytes) -> "WaveletBuffer":
        """
        Restore serialized buffer from string
        Args:
            blob: serialized buffer
        Returns:
            restored wavelet buffer
        """
        return impl.WaveletBuffer.parse(blob)

    def decompose(self, data: "np.ndarray[np.float32]", denoiser: BaseDenoiser) -> None:
        """
        Decompose signal with wavelet decomposition
        The signal should have a shape which was set in the constructor of the buffer

        Args:
            data: signal to decompose
            denoiser: denoiser algorithm
        Returns:
            RuntimeError if input data isn't valid
        """
        super(WaveletBuffer, self).decompose(data, denoiser)

    def compose(self, scale_factor: int = 0) -> "np.ndarray[np.float32]":
        """
        Compose signal from subbands with given scale factor
        Args:
            scale_factor: who many steps of wavelet decomposition should be re-compose
        Returns:
            restored signals
        """
        return super(WaveletBuffer, self).compose(scale_factor)

    def serialize(self, compression_level: int = 0) -> bytes:
        """
        Serialize and compress buffer to binary string
        Args:
            compression_level: 0- no compression, 16 max compression (bfloat for each value)
        Returns:
            serialized and compressed buffer
        """
        return super(WaveletBuffer, self).serialize(compression_level)

    @property
    def parameters(self) -> WaveletParameters:
        """
        Returns:
            current wavelet parameters
        """
        return super(WaveletBuffer, self).parameters
