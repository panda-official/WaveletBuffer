// Copyright 2020-2021 PANDA GmbH
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <wavelet_buffer/wavelet_parameters.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using drift::WaveletParameters;
using drift::WaveletTypes;

std::string WaveletTypeRepr(WaveletTypes value);

std::string WaveletParametersRepr(const std::string &class_name,
                                  const WaveletParameters &params) {
  std::stringstream ss;
  ss << class_name << "<"
     << "signal_number=" << params.signal_number << ", signal_shape=(";
  for (int i = 0; i < params.signal_shape.size(); i++) {
    if (i) {
      ss << "x";
    }
    ss << params.signal_shape[i];
  }
  ss << "), decomposition_steps=" << params.decomposition_steps
     << ", wavelet_type=" << WaveletTypeRepr(params.wavelet_type) << ">";
  return ss.str();
}

std::string WaveletTypeRepr(WaveletTypes value) {
  switch (value) {
    case WaveletTypes::kNone: {
      return "WaveletType.NONE";
    }
    case WaveletTypes::kDB1: {
      return "WaveletType.DB1";
    }
    case WaveletTypes::kDB2: {
      return "WaveletType.DB2";
    }
    case WaveletTypes::kDB3: {
      return "WaveletType.DB3";
    }
    case WaveletTypes::kDB4: {
      return "WaveletType.DB4";
    }
    case WaveletTypes::kDB5: {
      return "WaveletType.DB5";
    }
    default:
      return "Unknown";
  }
}

void WrapEnums(py::module *m) {
  py::enum_<WaveletTypes>(*m, "WaveletType")
      .value("NONE", WaveletTypes::kNone)
      .value("DB1", WaveletTypes::kDB1)
      .value("DB2", WaveletTypes::kDB2)
      .value("DB3", WaveletTypes::kDB3)
      .value("DB4", WaveletTypes::kDB4)
      .value("DB5", WaveletTypes::kDB5);
}
void WrapWaveletParameters(py::module *m) {
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
