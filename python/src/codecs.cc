// Copyright 2020-2022 PANDA GmbH

#include "codecs.h"

#include <wavelet_buffer/codecs/image_codec.h>
#include <wavelet_buffer/codecs/jpeg_codecs.h>

#include <pybind11/numpy.h>
#include <pybind11/operators.h>

#include "blaze_utils.h"


void WrapCodecAlgorithms(py::module* m) {
  using drift::SignalN2D;
  using drift::img::codecs::ColorSpace;
  using drift::img::codecs::GrayJpegCodec;
  using drift::img::codecs::HslJpegCodec;
  using drift::img::codecs::IImageCodec;
  using drift::img::codecs::RgbJpegCodec;

  auto module = m->def_submodule("codecs", "Codecs");

  module.doc() =
      "Codecs to encode and decode pictures to\\from numpy images."
      "Numpy image has planar layout: shape = [channel_number, height, width]";

  auto base = py::class_<IImageCodec>(module, "Base");

  auto decode = [](IImageCodec& self, const py::bytes& data) {
    SignalN2D image;
    if (!self.Decode(data, &image, 0)) {
      throw py::value_error("Failed to decode data");
    }

    return SignalN2DToPyArray(image);
  };

  auto encode = [](IImageCodec& self, const py::array& data,
                   size_t start_channel) {
    auto shape = data.shape();
    std::vector signal_shape = {static_cast<size_t>(shape[1]),
                                static_cast<size_t>(shape[2])};
    std::string blob;
    if (!self.Encode(PyArrayToSignalN2D(data), &blob, start_channel)) {
      throw py::value_error("Failed to encode data");
    }
    return py::bytes(blob);
  };

  base.def("decode", decode, "Decodes pictures from string to a numpy image",
           py::arg("data"));

  base.def("encode", encode,
           "Encodes a numpy image to string in some image format",
           py::arg("data"), py::arg("start_channel") = 0);

  base.def("channel_number",
           [](IImageCodec& self) { return self.channel_number(); });

  py::class_<RgbJpegCodec, IImageCodec>(module, "RgbJpeg")
      .def(py::init(
               [](float write_quality) { return RgbJpegCodec(write_quality); }),
           py::arg("write_quality") = 1.f);

  py::class_<HslJpegCodec, IImageCodec>(module, "HslJpeg")
      .def(py::init(
               [](float write_quality) { return HslJpegCodec(write_quality); }),
           py::arg("write_quality") = 1.f);

  py::class_<GrayJpegCodec, IImageCodec>(module, "GrayJpeg")
      .def(py::init([](float write_quality) {
             return GrayJpegCodec(write_quality);
           }),
           py::arg("write_quality") = 1.f);
}
