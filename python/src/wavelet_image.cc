// Copyright 2020-2022 PANDA GmbH

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <wavelet_buffer/wavelet_image.h>
#include <wavelet_buffer/wavelet_parameters.h>

#include <memory>

#include "wavelet_buffer/codecs/image_codec.h"
#include "wavelet_buffer/denoise_algorithms.h"

namespace py = pybind11;

using drift::WaveletBuffer;
using drift::WaveletImage;
using drift::WaveletParameters;
using drift::WaveletTypes;
using Denoiser = drift::DenoiseAlgorithm<drift::DataType>;
using Codec = drift::codecs::IImageCodec;

//std::string WaveletTypeRepr(WaveletTypes value);

std::string WaveletParametersRepr(const std::string &class_name,
                                  const WaveletParameters &params);

void WrapWaveletImage(py::module *m) {
  auto cls = py::class_<WaveletImage>(*m, "WaveletImage");
  cls.def(py::init([](const std::vector<size_t> &signal_shape,
                      size_t channel_number, size_t decomposition_steps,
                      WaveletTypes wavelet_type) {
            return std::make_unique<WaveletImage>(WaveletParameters{
                .signal_shape = signal_shape,
                .signal_number = channel_number,
                .decomposition_steps = decomposition_steps,
                .wavelet_type = wavelet_type,
            });
          }),
          py::arg("signal_shape"), py::arg("signal_number"),
          py::arg("decomposition_steps"), py::arg("wavelet_type"));

  cls.def(py::init<WaveletBuffer>(), py::arg("buffer"));

  cls.def(
      "import_from_file",
      [](WaveletImage &self, const std::string &file_path,
         const Denoiser &denoiser, const Codec &codec, size_t start_channel) {
        auto status =
            self.ImportFromFile(file_path, denoiser, codec, start_channel);
        if (status != WaveletImage::Status::kOk) {
          throw py::value_error("Unable to import image from file. Error " +
                                std::to_string(status));
        }
      },
      py::arg("file_path"), py::arg("denoiser"), py::arg("codec"),
      py::arg("start_channel") = 0);

  cls.def(
      "export_to_file",
      [](const WaveletImage &self, const std::string &file_path,
         const Codec &codec, size_t start_channel) {
        auto status = self.ExportToFile(file_path, codec, start_channel);
        if (status != WaveletImage::Status::kOk) {
          throw py::value_error("Unable to export image to file");
        }
      },
      py::arg("file_path"), py::arg("codec"), py::arg("start_channel") = 0);

  cls.def(
      "import_from_string",
      [](WaveletImage &self, const std::string &data, const Denoiser &denoiser,
         const Codec &codec, size_t start_channel) {
        auto status =
            self.ImportFromString(data, denoiser, codec, start_channel);
        if (status != WaveletImage::Status::kOk) {
          throw py::value_error("Unable to import image");
        }
      },
      py::arg("data"), py::arg("denoiser"), py::arg("codec"),
      py::arg("start_channel") = 0);

  cls.def(
      "export_to_string",
      [](const WaveletImage &self, const WaveletImage::Codec &codec,
         size_t start_channel) {
        std::string result;
        auto status = self.ExportToString(&result, codec, start_channel);
        if (status != WaveletImage::Status::kOk) {
          throw py::value_error("Unable to export image");
        }
        return py::bytes(result);
      },
      py::arg("codec"), py::arg("start_channel") = 0);

  cls.def_static("load", &WaveletImage::Load, py::arg("file_path"));

  cls.def(
      "save",
      [](const WaveletImage &self, const std::string &file_path) {
        auto status = self.Save(file_path);
        if (status != WaveletImage::Status::kOk) {
          throw py::value_error("Unable to save image");
        }
      },
      py::arg("file_path"));

  cls.def("distance", [](const WaveletImage &a, const WaveletImage &b) {
    return a.CompareTo(b);
  });

  cls.def_property_readonly("buffer", &WaveletImage::buffer);

  py::enum_<WaveletImage::Status>(cls, "Status")
      .value("Ok", WaveletImage::Status::kOk)
      .value("WrongSize", WaveletImage::Status::kWrongSize)
      .value("DecompositionError", WaveletImage::Status::kDecompositionError)
      .value("CompositionError", WaveletImage::Status::kCompositionError)
      .value("IOError", WaveletImage::Status::kIOError)
      .value("WrongData", WaveletImage::Status::kWrongData);

  py::class_<WaveletParameters>(*m, "WaveletParameters")
      .def_readonly("signal_number", &WaveletParameters::signal_number)
      .def_readonly("decomposition_steps",
                    &WaveletParameters::decomposition_steps)
      .def_readonly("signal_shape", &WaveletParameters::signal_shape)
      .def_readonly("wavelet_type", &WaveletParameters::wavelet_type)

      .def("__repr__", [](const WaveletParameters &self) {
        return WaveletParametersRepr("WaveletParameters", self);
      });
}
