"""WaveletBuffer"""
import wavelet_buffer._wavelet_buffer as impl

from wavelet_buffer.denoise import BaseDenoiser


class WaveletBufferView(impl.WaveletBufferView):
    """View on slice of signals in WaveletBuffer"""

    def __int__(self):
        raise NotImplemented("Can't be created only with WaveletBuffer.__item__")

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
        super(WaveletBufferView, self).decompose(data, denoiser)

    def compose(self, scale_factor: int = 0) -> "np.ndarray[np.float32]":
        """
        Compose signal from subbands with given scale factor
        Args:
            scale_factor: who many steps of wavelet decomposition should be re-compose
        Returns:
            restored signals
        """
        return super(WaveletBufferView, self).compose(scale_factor)
