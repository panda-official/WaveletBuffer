"""Collections of image codecs"""
import wavelet_buffer._wavelet_buffer as impl


class BaseCodec:
    """Interface for codes to decode/encode images from/to different image formats"""

    def decode(self, blob: bytes) -> "np.ndarray[np.float32]":
        """Decode an image from bytes to a numpy image
        Args:
            blob: serialized image e.g. JPEG
        Returns:
            image as numpy array
        """
        raise NotImplemented()

    def encode(self, data: "np.ndarray[np.float32]", start_channel: int = 0) -> bytes:
        """Encode a numpy image to byte string
        Args:
            data: numpy image
            start_channel: start channel for conversion
        Returns:
            serialized image e.g. JPEG
        """
        raise NotImplemented()

    def channel_number(self) -> int:
        """Return number ofr channel RGB=3, Gray=1"""
        raise NotImplemented()


class RgbJpeg(BaseCodec):
    """Codec for JPEG <-> numpy RGB image conversion"""

    def __init__(self, write_quality: float = 1.0):
        """
        Args:
            write_quality: quality of encoded image. Should be from 0 to 1.0
        """
        self._impl = impl.RgbJpeg(write_quality)

    def decode(self, blob: bytes) -> "np.ndarray[np.float32]":
        return self._impl.decode(blob)

    def encode(self, data: "np.ndarray[np.float32]", start_channel: int = 0) -> bytes:
        return self._impl.encode(data, start_channel)

    def channel_number(self) -> int:
        return self._impl.channel_number()


class HslJpeg(BaseCodec):
    """Codec for JPEG <-> numpy HSL image conversion"""

    def __init__(self, write_quality: float = 1.0):
        """
        Args:
            write_quality: quality of encoded image. Should be from 0 to 1.0
        """
        self._impl = impl.HslJpeg(write_quality)

    def decode(self, blob: bytes) -> "np.ndarray[np.float32]":
        return self._impl.decode(blob)

    def encode(self, data: "np.ndarray[np.float32]", start_channel: int = 0) -> bytes:
        return self._impl.encode(data, start_channel)

    def channel_number(self) -> int:
        return self._impl.channel_number()


class GrayJpeg(BaseCodec):
    """Codec for JPEG  <-> numpy monochrome image conversion"""

    def __init__(self, write_quality: float = 1.0):
        """
        Args:
            write_quality: quality of encoded image. Should be from 0 to 1.0
        """
        self._impl = impl.GrayJpeg(write_quality)

    def decode(self, blob: bytes) -> "np.ndarray[np.float32]":
        return self._impl.decode(blob)

    def encode(self, data: "np.ndarray[np.float32]", start_channel: int = 0) -> bytes:
        return self._impl.encode(data, start_channel)

    def channel_number(self) -> int:
        return self._impl.channel_number()
