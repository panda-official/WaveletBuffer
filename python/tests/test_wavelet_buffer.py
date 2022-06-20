"""
Module test image buffer
"""
import re

import numpy as np
import pytest

from wavelet_buffer import (
    WaveletBuffer,
    WaveletType,
    distance,
    energy_distribution,
)  # pylint: disable=no-name-in-module
from wavelet_buffer import denoise  # pylint: disable=no-name-in-module


def parameters_2d() -> dict:
    """Get wavelet parameters for multichannel 2D signal"""
    return dict(
        signal_shape=[40, 20],
        signal_number=3,
        decomposition_steps=2,
        wavelet_type=WaveletType.DB2,
    )


def parameters_1d() -> dict:
    """Get wavelet parameters for 1D signal"""
    return dict(
        signal_shape=[100],
        signal_number=1,
        decomposition_steps=2,
        wavelet_type=WaveletType.DB2,
    )


mark_both_dimensions = pytest.mark.parametrize(
    "params", [parameters_1d(), parameters_2d()]
)

mark_2d_dimension = pytest.mark.parametrize("params", [parameters_2d()])

mark_1d_dimension = pytest.mark.parametrize("params", [parameters_1d()])


@pytest.fixture(name="input_signal")
def fixture_input_signal():
    """Construct input signal that match specific wavelet parameters"""

    def generate(params):
        if len(params["signal_shape"]) > 1:
            width, height = params["signal_shape"]
            return np.array(
                [[list(range(width))] * height] * params["signal_number"],
                dtype=np.float32,
            )
        (size,) = params["signal_shape"]
        return np.array(list(range(size)), dtype=np.float32)

    return generate


@pytest.fixture(name="buffer")
def decomposed_buffer(params, input_signal):
    """Make buffer and decompose signal"""
    signal = input_signal(params)
    buffer = WaveletBuffer(**params)
    buffer.decompose(signal, denoise.Null())

    return buffer


@mark_both_dimensions
def test__construction_from_subbands(buffer, params):
    """should be created with decomposition"""
    new_buffer = WaveletBuffer(decompositions=buffer.decompositions, **params)

    assert distance(buffer, new_buffer) == 0


@mark_both_dimensions
def test__decomposition_size(params):
    """decomposition size should positive and non zero"""
    assert WaveletBuffer.decomposition_size(**params) > 0


@mark_both_dimensions
def test__compose_decompose(buffer, params, input_signal):
    """decomposed and composed back signal should be close to initial"""
    signal = input_signal(params)

    restored_signal = np.array(buffer.compose())
    assert np.all(np.abs(signal - restored_signal) < 0.01)


@mark_both_dimensions
def test__compose_decompose_with_scale_factor1(buffer, params, input_signal):
    """decomposed and composed back signal should be twice smaller"""
    signal = input_signal(params)
    restored_signal = np.array(buffer.compose(scale_factor=1))
    assert np.prod(restored_signal.shape) == np.prod(signal.shape) / (
        2 * len(params["signal_shape"])
    )


# @mark_both_dimensions
# def test__compose_with_greater_scale_factor(buffer, params):
#     """ should not compose back signal with wrong scale factor
#     """
#     with pytest.raises(BufferError) as err:
#         buffer.compose(scale_factor=params['decomposition_steps'] + 1)
#     assert str(err.value) == 'Failed to compose data'


def test__compose_2d__wrong_shape():
    """decompose should fail for signals with incorrect shape"""
    params = parameters_2d()
    (width, height) = params["signal_shape"]
    channels = params["signal_number"]
    buffer = WaveletBuffer(**params)
    denoiser = denoise.Null()

    with pytest.raises(BufferError) as err:
        buffer.decompose(np.array([]), denoiser)
    assert str(err.value) == f"input data must have {channels} dim (actual 1)"

    with pytest.raises(BufferError) as err:
        buffer.decompose(np.array([[[]]]), denoiser)
    assert str(err.value) == f"input data must have {channels} channels (actual 1)"

    with pytest.raises(BufferError) as err:
        buffer.decompose(np.array([[[]]] * params["signal_number"]), denoiser)
    assert str(err.value) == f"input data must have {height} height (actual 1)"

    with pytest.raises(BufferError) as err:
        buffer.decompose(np.array([[[]] * height] * channels), denoiser)
    assert str(err.value) == f"input data must have {width} width (actual 0)"


def test__compose_1d__wrong_shape():
    """decompose should fail for signals with incorrect shape"""
    params = parameters_1d()
    (size,) = params["signal_shape"]
    buffer = WaveletBuffer(**params)

    with pytest.raises(BufferError) as err:
        buffer.decompose(np.array([[1]] * 2), denoise.Null())
    assert str(err.value) == "input data must have 1 dim (actual 2)"

    with pytest.raises(BufferError) as err:
        buffer.decompose(np.array([]), denoise.Null())
    assert str(err.value) == f"input data must be {size} items long (actual 0)"

    with pytest.raises(BufferError) as err:
        buffer.decompose(np.array([1, 2]), denoise.Null())
    assert str(err.value) == f"input data must be {size} items long (actual 2)"


@mark_both_dimensions
def test__serialize_parse(buffer):
    """serialized and then deserialized buffer should be equal to initial"""
    blob = buffer.serialize()
    assert len(blob) > 0

    restored_buffer = WaveletBuffer.parse(blob)
    assert restored_buffer is not None
    assert restored_buffer == buffer
    assert distance(buffer, restored_buffer) == 0


@mark_both_dimensions
def test__serialize_with_compression(params, input_signal):
    """should serialize with compression"""
    buffer = WaveletBuffer(**params)
    buffer.decompose(input_signal(params), denoise.Simple(0.9))

    assert len(buffer.serialize()) > len(buffer.serialize(compression_level=16)) * 3


@mark_both_dimensions
def test__subbands_access(buffer):
    """subbands should be accessible through index operator"""
    subbands = buffer.decompositions[0]

    assert len(subbands) > 0


@mark_both_dimensions
def test__subbands_get_last_approximation(params, input_signal):
    """buffer without compression should contain original signal"""
    params["decomposition_steps"] = 0
    signal = input_signal(params)
    buffer = WaveletBuffer(**params)
    buffer.decompose(signal, denoise.Null())

    for i in range(params["signal_number"]):
        assert len(buffer.decompositions[i]) == 1


@mark_both_dimensions
def test__parameters(params):
    """buffer should return same parameters that were used during creation"""
    buffer = WaveletBuffer(**params)

    shape_str = "x".join(map(str, params["signal_shape"]))
    expected_str = (
        "WaveletParameters<"
        "signal_number={signal_number}, signal_shape=({shape_str}), "
        "decomposition_steps={decomposition_steps}, "
        "wavelet_type={wavelet_type}>".format(**params, shape_str=shape_str)
    )

    with pytest.raises(AttributeError, match="can't set attribute"):
        buffer.parameters = 10

    ro_params = buffer.parameters  # but can read

    assert ro_params.signal_shape == params["signal_shape"]
    assert ro_params.signal_number == params["signal_number"]
    assert ro_params.decomposition_steps == params["decomposition_steps"]
    assert ro_params.wavelet_type == params["wavelet_type"]

    assert str(ro_params) == expected_str


@mark_2d_dimension
def test__slicing(params, input_signal):
    """buffer can be sliced by signals that we decompose\\compose
    separately
    """
    buffer = WaveletBuffer(**params)
    signal = input_signal(params)

    view = buffer[1:2]
    view.decompose(signal[0:1], denoise.Null())

    restored_signal = view.compose(scale_factor=0)
    assert np.all(np.abs(signal - restored_signal) < 0.01)


@mark_2d_dimension
def test__slicing_error_check(params, input_signal):
    """buffer's slice should check the form of the  input signal"""
    buffer = WaveletBuffer(**params)
    signal = input_signal(params)

    with pytest.raises(BufferError, match=re.escape("20 height (actual 40)")):
        view = buffer[1:2]
        signal = np.reshape(signal, (signal.shape[0], signal.shape[2], signal.shape[1]))
        view.decompose(signal[0:1], denoise.Null())

    with pytest.raises(ValueError, match="Step > 1 is not supported"):
        _ = buffer[0:4:2]

    with pytest.raises(BufferError, match=re.escape("must have 3 channels (actual 2)")):
        view = buffer[0:4]
        view.decompose(signal[0:2], denoise.Null())


@mark_1d_dimension
def test__slicing_does_not_support_1d(params, input_signal):
    """slicing works only for 2d signals"""
    buffer = WaveletBuffer(**params)
    signal = input_signal(params)

    with pytest.raises(BufferError, match="dimension"):
        _ = buffer[0:1].decompose(signal, denoise.Null())


def test__none_decomposition_for_small_signals():
    """WaveletType.NONE to switch off the decomposition"""
    buffer = WaveletBuffer(
        signal_shape=[2, 2],
        signal_number=1,
        decomposition_steps=0,
        wavelet_type=WaveletType.NONE,
    )
    signal = np.array([[[1, 2], [2, 3]]], dtype=np.float32)
    buffer.decompose(signal, denoise.Null())
    assert np.all(np.abs(signal - buffer.compose()) < 0.01)


@mark_both_dimensions
def test__is_empty_method(params, input_signal):
    """should check that decompositions initialized"""
    signal = input_signal(params)
    buffer = WaveletBuffer(**params)
    assert buffer.is_empty()

    buffer.decompose(signal, denoise.Null())

    assert not buffer.is_empty()


@mark_both_dimensions
def test__is_empty_after_serialisation(params):
    """should serialize/deserialize empty buffer"""
    buffer = WaveletBuffer(**params)
    assert buffer.is_empty()

    blob = buffer.serialize()

    restored_buffer = WaveletBuffer.parse(blob)
    assert restored_buffer.is_empty()


@mark_1d_dimension
def test__energy_distribution_1d(buffer):
    """should calculate energy distribution for 1d signal"""
    assert np.all(
        energy_distribution(buffer) - np.array([1250.0, 2313.1018, 324786.94]) < 0.01
    )


@mark_2d_dimension
def test__energy_distribution_2d(buffer):
    """should calculate energy distribution for 2d signal"""
    assert np.all(
        energy_distribution(buffer)
        - np.array(
            [
                [
                    1.9237234e-10,
                    4.0000000e03,
                    2.2737368e-12,
                    3.5494452e-10,
                    7.4019268e03,
                    5.7002656e-11,
                    3.9939819e05,
                ]
            ]
            * 3
        )
        < 0.01
    )
