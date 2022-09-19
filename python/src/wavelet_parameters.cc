// Copyright 2020-2022 PANDA GmbH

#include "src/wavelet_parameters.h"

#include <pybind11/stl.h>

std::string WaveletTypeRepr(drift::WaveletTypes value) {
  switch (value) {
    case drift::WaveletTypes::kNone: {
      return "WaveletType.NONE";
    }
    case drift::WaveletTypes::kDB1: {
      return "WaveletType.DB1";
    }
    case drift::WaveletTypes::kDB2: {
      return "WaveletType.DB2";
    }
    case drift::WaveletTypes::kDB3: {
      return "WaveletType.DB3";
    }
    case drift::WaveletTypes::kDB4: {
      return "WaveletType.DB4";
    }
    case drift::WaveletTypes::kDB5: {
      return "WaveletType.DB5";
    }
    default:
      return "Unknown";
  }
}

std::string WaveletParametersRepr(const std::string &class_name,
                                  const drift::WaveletParameters &params) {
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

void WrapWaveletType(py::module *m) {
  py::enum_<drift::WaveletTypes>(*m, "WaveletType")
      .value("NONE", drift::WaveletTypes::kNone)
      .value("DB1", drift::WaveletTypes::kDB1)
      .value("DB2", drift::WaveletTypes::kDB2)
      .value("DB3", drift::WaveletTypes::kDB3)
      .value("DB4", drift::WaveletTypes::kDB4)
      .value("DB5", drift::WaveletTypes::kDB5);
}

void WrapWaveletParameters(py::module *m) {
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
