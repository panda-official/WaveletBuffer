from wavelet_buffer import (
    WaveletType,
    denoise,
)
from wavelet_buffer.img import WaveletImage, codecs

params = dict(
    signal_shape=[800, 500],
    signal_number=3,
    decomposition_steps=6,
    wavelet_type=WaveletType.DB5,
)

img = WaveletImage(**params)

status = img.import_from_file(
    "../../test/fixtures/pandas.jpg",
    denoise.Simple(compression_level=0.7),
    codecs.HslJpeg(),
)

img.save("output.bin")
img.export_to_file("output.jpeg", codec=codecs.HslJpeg())
img2 = WaveletImage.load("output.bin")
print("distance between images", WaveletImage.distance(img, img2))

img3 = WaveletImage(**params)
img3.import_from_file(
    "output.jpeg",
    denoiser=denoise.Simple(compression_level=0.3),
    codec=codecs.RgbJpeg(),
)

print("distance between images", WaveletImage.distance(img, img3))
