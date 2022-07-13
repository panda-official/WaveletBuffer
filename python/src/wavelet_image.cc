// Copyright 2020-2022 PANDA GmbH

#include "wavelet_image.h"

//#include <pybind11/operators.h>
#include <wavelet_buffer/codecs/image_codec.h>
#include <wavelet_buffer/denoise_algorithms.h>
#include <wavelet_buffer/wavelet_image.h>
#include <wavelet_buffer/wavelet_parameters.h>

#include <memory>

#include "wavelet_parameters.h"

namespace py = pybind11;

using Denoiser = drift::DenoiseAlgorithm<drift::DataType>;

void WrapWaveletImage(py::module *m) {
  auto cls = py::class_<drift::WaveletImage>(*m, "WaveletImage");
  cls.def(
      py::init([](const std::vector<size_t> &signal_shape,
                  size_t channel_number, size_t decomposition_steps,
                  drift::WaveletTypes wavelet_type) {
        return std::make_unique<drift::WaveletImage>(drift::WaveletParameters{
            .signal_shape = signal_shape,
            .signal_number = channel_number,
            .decomposition_steps = decomposition_steps,
            .wavelet_type = wavelet_type,
        });
      }),
      py::arg("signal_shape"), py::arg("signal_number"),
      py::arg("decomposition_steps"), py::arg("wavelet_type"));

  cls.def(py::init<drift::WaveletBuffer>(), py::arg("buffer"));

  cls.def(
      "import_from_file",
      [](drift::WaveletImage &self, const std::string &file_path,
         const Denoiser &denoiser, const drift::WaveletImage::Codec &codec, size_t start_channel) {
        auto status =
            self.ImportFromFile(file_path, denoiser, codec, start_channel);
        if (status != drift::WaveletImage::Status::kOk) {
          throw py::value_error("Unable to import image from file. Error " +
                                std::to_string(status));
        }
      },
      py::arg("file_path"), py::arg("denoiser"), py::arg("codec"),
      py::arg("start_channel") = 0);

  cls.def(
      "export_to_file",
      [](const drift::WaveletImage &self, const std::string &file_path,
         const drift::WaveletImage::Codec &codec, size_t start_channel) {
        auto status = self.ExportToFile(file_path, codec, start_channel);
        if (status != drift::WaveletImage::Status::kOk) {
          throw py::value_error("Unable to export image to file");
        }
      },
      py::arg("file_path"), py::arg("codec"), py::arg("start_channel") = 0);

  cls.def(
      "import_from_string",
      [](drift::WaveletImage &self, const std::string &data, const Denoiser &denoiser,
         const drift::WaveletImage::Codec &codec, size_t start_channel) {
        auto status =
            self.ImportFromString(data, denoiser, codec, start_channel);
        if (status != drift::WaveletImage::Status::kOk) {
          throw py::value_error("Unable to import image");
        }
      },
      py::arg("data"), py::arg("denoiser"), py::arg("codec"),
      py::arg("start_channel") = 0);

  cls.def(
      "export_to_string",
      [](const drift::WaveletImage &self, const drift::WaveletImage::Codec &codec,
         size_t start_channel) {
        std::string result;
        auto status = self.ExportToString(&result, codec, start_channel);
        if (status != drift::WaveletImage::Status::kOk) {
          throw py::value_error("Unable to export image");
        }
        return py::bytes(result);
      },
      py::arg("codec"), py::arg("start_channel") = 0);

  cls.def_static("load", &drift::WaveletImage::Load, py::arg("file_path"));

  cls.def(
      "save",
      [](const drift::WaveletImage &self, const std::string &file_path) {
        auto status = self.Save(file_path);
        if (status != drift::WaveletImage::Status::kOk) {
          throw py::value_error("Unable to save image");
        }
      },
      py::arg("file_path"));

  cls.def("distance", [](const drift::WaveletImage &a, const drift::WaveletImage &b) {
    return a.CompareTo(b);
  });

  cls.def_property_readonly("buffer", &drift::WaveletImage::buffer);

  py::enum_<drift::WaveletImage::Status>(cls, "Status")
      .value("Ok", drift::WaveletImage::Status::kOk)
      .value("WrongSize", drift::WaveletImage::Status::kWrongSize)
      .value("DecompositionError", drift::WaveletImage::Status::kDecompositionError)
      .value("CompositionError", drift::WaveletImage::Status::kCompositionError)
      .value("IOError", drift::WaveletImage::Status::kIOError)
      .value("WrongData", drift::WaveletImage::Status::kWrongData);

  py::class_<drift::WaveletParameters>(*m, "WaveletParameters")
      .def_readonly("signal_number", &drift::WaveletParameters::signal_number)
      .def_readonly("decomposition_steps",
                    &drift::WaveletParameters::decomposition_steps)
      .def_readonly("signal_shape", &drift::WaveletParameters::signal_shape)
      .def_readonly("wavelet_type", &drift::WaveletParameters::wavelet_type)

      .def("__repr__", [](const drift::WaveletParameters &self) {
        return WaveletParametersRepr("WaveletParameters", self);
      });
}
