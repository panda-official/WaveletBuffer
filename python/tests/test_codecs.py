"""
Module test codecs
"""
import io

import pytest
import numpy as np
from wavelet_buffer.img import (
    HslJpeg,
    RgbJpeg,
    GrayJpeg,
    BaseCodec,
)  # pylint: disable=no-name-in-module
from PIL import Image


@pytest.mark.parametrize("cls", [HslJpeg, RgbJpeg, GrayJpeg])
class TestJpegCodecs:  # pylint: disable=too-few-public-methods
    """
    Test JPEG
    """

    def test__constructor(self, cls):  # pylint: disable=no-self-use
        """
        Test constructor
        """
        with pytest.raises(RuntimeError, match="write_quality out of range"):
            cls(write_quality=2)
        with pytest.raises(RuntimeError, match="write_quality out of range"):
            cls(write_quality=-1)

        cls(write_quality=1)
        cls(write_quality=0.5)
        cls(write_quality=0)

    def test__encode(self, cls):  # pylint: disable=no-self-use
        """Test JPG encoding-decoding"""
        codec: BaseCodec = cls()

        channel = [[0.1, 0.1, 0.1], [0.5, 0.5, 0.5], [0.2, 0.2, 0.2], [1.0, 1.0, 1.0]]
        image = np.asarray(
            [[[0] * 3, [0] * 3, [0] * 3, [0] * 3], channel, channel, channel],
            dtype=np.single,
        )
        data: bytes = codec.encode(image, 1)
        assert len(data) > 300

        pil_image: Image = Image.open(io.BytesIO(data), formats=("JPEG",))
        assert pil_image

        decoded_image = codec.decode(data)
        print(cls)
        print(decoded_image)
        print(image[1 : codec.channel_number() + 1])

        assert np.linalg.norm(image[1 : codec.channel_number() + 1]) == pytest.approx(
            np.linalg.norm(decoded_image), 0.5
        )
