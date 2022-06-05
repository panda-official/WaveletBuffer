// Copyright 2020 PANDA GmbH
#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>

#include <vector>

namespace py = pybind11;

extern void ExportBlazeMatrixes(py::module *m);

PYBIND11_MAKE_OPAQUE(std::vector<int64_t>);

void ExportVectors(py::module *m) {
  py::bind_vector<std::vector<int64_t>>(*m, "LongVector", py::buffer_protocol())
      .def("ptr", [](std::vector<int64_t> &self) {
        return reinterpret_cast<uint64_t>(&self);
      });
  py::bind_vector<std::vector<double>>(*m, "DoubleVector",
                                       py::buffer_protocol());
  py::bind_vector<std::vector<float>>(*m, "FloatVector", py::buffer_protocol());
}

PYBIND11_MODULE(_wb, m) {
  ExportBlazeMatrixes(&m);
  ExportVectors(&m);
}
