"""Helper module to use WaveletBuffer with images"""
from typing import List
import wavelet_buffer._wavelet_buffer as impl

from wavelet_buffer.denoise import BaseDenoiser
from wavelet_buffer.img.codecs import BaseCodec
from wavelet_buffer.wavelet_buffer import WaveletType, WaveletBuffer


class WaveletImage(impl.WaveletImage):
    """A class to load/save images of different formats and store them in WaveletBuffer"""

    def __init__(
        self,
        signal_shape: List[int],
        signal_number: int,
        decomposition_steps: int,
        wavelet_type: WaveletType,
    ):
        """
        Args:
            signal_shape: for 1D signal with size N [N], for 2DN signal with size MXN, [M,N]
            signal_number: N for 2DN signal, e.g. RGB image would be 3 signals
            decomposition_steps: number of decomposition steps
            wavelet_type: type WaveletType.NONE, DB{1..5}, if it is NONE no wavelet composition
        """
        super().__init__(signal_shape, signal_number, decomposition_steps, wavelet_type)

    def import_from_file(
        self,
        file_path: str,
        denoiser: BaseDenoiser,
        codec: BaseCodec,
        start_channel: int = 0,
    ) -> None:
        super(WaveletImage, self).import_from_file(
            file_path, denoiser, codec._impl, start_channel
        )

    def export_to_file(
        self, file_path: str, codec: BaseCodec, start_channel: int = 0
    ) -> None:
        super(WaveletImage, self).export_to_file(file_path, codec._impl, start_channel)

    def import_from_string(
        self,
        data: bytes,
        denoiser: BaseDenoiser,
        codec: BaseCodec,
        start_channel: int = 0,
    ) -> None:
        super(WaveletImage, self).import_from_string(
            data, denoiser, codec._impl, start_channel
        )

    def export_to_string(self, codec: BaseCodec, start_channel: int = 0) -> bytes:
        return super(WaveletImage, self).export_to_string(codec._impl, start_channel)

    @staticmethod
    def load(file_path: str) -> "WaveletImage":
        return impl.WaveletImage.load(file_path)

    def save(self, file_path: str) -> None:
        super(WaveletImage, self).save(file_path)

    def distance(self, other: "WaveletImage") -> float:
        return super(WaveletImage, self).distance(other)

    @property
    def buffer(self) -> WaveletBuffer:
        return super(WaveletImage, self).buffer
