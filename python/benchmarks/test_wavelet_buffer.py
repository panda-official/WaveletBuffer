from pytest import fixture
import numpy as np
from wavelet_buffer import WaveletBuffer, WaveletType, denoise


@fixture(name="input_matrix")
def _make_input():
    return np.array([np.random.random((1080, 1920))], dtype=np.float32)


@fixture(name="buffer")
def _make_buffer() -> WaveletBuffer:
    return WaveletBuffer(
        signal_shape=[1920, 1080],
        signal_number=1,
        decomposition_steps=5,
        wavelet_type=WaveletType.DB3,
    )


def test_decompose(benchmark, buffer, input_matrix):
    benchmark.pedantic(buffer.decompose, args=(input_matrix, denoise.Null()), rounds=50)
    assert True


def test_compose(benchmark, buffer, input_matrix):
    buffer.decompose(input_matrix, denoise.Null())
    benchmark.pedantic(buffer.compose, rounds=50)
    assert True
