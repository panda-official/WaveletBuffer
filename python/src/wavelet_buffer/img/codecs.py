"""Collections of image codecs"""
import wavelet_buffer._wavelet_buffer as impl


class BaseCodec(impl.BaseCodec):
    """Interface for codes to decode/encode images from/to different image formats"""

    def __init__(self):
        super().__init__()

    def decode(self, blob: bytes) -> "np.ndarray[np.float32]":
        """Decode an image from bytes to a numpy image
        Args:
            blob: serialized image e.g. JPEG
        Returns:
            image as numpy array
        """
        raise NotImplemented()

    def encode(self, data: "np.ndarray[np.float32]") -> bytes:
        """Encode a numpy image to byte string
        Args:
            data: numpy image
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
        _impl = impl.RgbJpeg(write_quality)
        super().__init__()

    def decode(self, blob: bytes) -> "np.ndarray[np.float32]":
        return _imp.decode(blob)

    def encode(self, data: "np.ndarray[np.float32]") -> bytes:
        return _imp.encode(data)

    def channel_number(self) -> int:
        return _imp.channel_number()


class HslJpeg(BaseCodec):
    """Codec for JPEG <-> numpy HSL image conversion"""

    def __init__(self, write_quality: float = 1.0):
        """
        Args:
            write_quality: quality of encoded image. Should be from 0 to 1.0
        """
        _impl = impl.HslJpeg(write_quality)
        super().__init__()

    def decode(self, blob: bytes) -> "np.ndarray[np.float32]":
        return _imp.decode(blob)

    def encode(self, data: "np.ndarray[np.float32]") -> bytes:
        return _imp.encode(data)

    def channel_number(self) -> int:
        return _imp.channel_number()


class GrayJpeg(BaseCodec):
    """Codec for JPEG  <-> numpy monochrome image conversion"""

    def __init__(self, write_quality: float = 1.0):
        """
        Args:
            write_quality: quality of encoded image. Should be from 0 to 1.0
        """
        _impl = impl.GrayJpeg(write_quality)
        super().__init__()

    def decode(self, blob: bytes) -> "np.ndarray[np.float32]":
        return _imp.decode(blob)

    def encode(self, data: "np.ndarray[np.float32]") -> bytes:
        return _imp.encode(data)

    def channel_number(self) -> int:
        return _imp.channel_number()
