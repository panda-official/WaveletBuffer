""" ImageWavelet Tests
"""

import pathlib

import pytest
from wavelet_buffer import (  # pylint: disable=no-name-in-module
    WaveletBuffer,
    WaveletImage,
    WaveletType,
    denoise,
    codecs,
)

IMAGE_PATH = str(pathlib.Path(__file__).parent.absolute() / "fixtures" / "img.jpeg")
NO_IMAGE_PATH = str(
    pathlib.Path(__file__).parent.absolute() / "fixtures" / "noimg.jpeg"
)


@pytest.fixture(name="wavelet_params")
def fixture_wavelet_params():
    """Wavelet Parameters Fixture"""
    return dict(
        signal_shape=(800, 500),
        signal_number=3,
        decomposition_steps=2,
        wavelet_type=WaveletType.DB3,
    )


@pytest.fixture(name="wavelet_image_with_data")
def fixture_wavelet_image_with_data(wavelet_params):
    """WaveletImage with loaded image"""
    img = WaveletImage(**wavelet_params)
    img.import_from_file(IMAGE_PATH, denoiser=denoise.Null(), codec=codecs.HslJpeg())
    return img


@pytest.mark.parametrize(
    "filepath,exception_catch",
    [("file_not_found", True), (NO_IMAGE_PATH, True), (IMAGE_PATH, False)],
)
def test__import_from_file(wavelet_params, filepath, exception_catch):
    """import from file should return expected status"""
    img = WaveletImage(**wavelet_params)
    try:
        img.import_from_file(filepath, denoiser=denoise.Null(), codec=codecs.HslJpeg(1))
    except ValueError:
        assert exception_catch


@pytest.mark.parametrize(
    "filename,codec,exception_catch",
    [
        ("output.jpg", codecs.HslJpeg(1), False),
        ("#$%!.jpg", codecs.HslJpeg(1), False),
        ("output.jpg", codecs.HslJpeg(0), False),
    ],
)
def test__export_to_file(wavelet_params, tmp_path, filename, codec, exception_catch):
    """export to file should not raise exception if ok"""
    filename = str(tmp_path / filename)
    img = WaveletImage(**wavelet_params)
    try:
        img.export_to_file(filename, codec=codec)
    except ValueError:
        assert exception_catch


def test__init_buffer(wavelet_params):
    """Image's buffer must be equal to buffer with same parameters"""
    img = WaveletImage(**wavelet_params)
    assert img.buffer == WaveletBuffer(**wavelet_params)


def test__import_export_from_string(wavelet_image_with_data, wavelet_params):
    """export to string and import from it later should result in equal
    wavelet buffer
    """
    string = wavelet_image_with_data.export_to_string(codec=codecs.HslJpeg())
    assert string

    restored_image = WaveletImage(**wavelet_params)
    restored_image.import_from_string(
        string, denoiser=denoise.Null(), codec=codecs.HslJpeg()
    )
    assert restored_image.distance(wavelet_image_with_data) < 0.006


def test__save_load(tmp_path, wavelet_image_with_data):
    """save to file and load from it later should result in equal wavelet buffer"""
    wavelet_image_with_data.save(str(tmp_path / "output.file"))

    restored_image = WaveletImage.load(str(tmp_path / "output.file"))
    assert restored_image.buffer == wavelet_image_with_data.buffer


def test__distance(wavelet_image_with_data):
    """distance for equal images should be zero"""
    assert wavelet_image_with_data.distance(wavelet_image_with_data) == 0
