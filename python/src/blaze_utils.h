// Copyright 2021 PANDA GmbH
#ifndef SRC_DSP_BLAZE_UTILS_H_
#define SRC_DSP_BLAZE_UTILS_H_

#include <wavelet_buffer/primitives.h>
#include <wavelet_buffer/wavelet_utils.h>

#include <pybind11/numpy.h>

#include <vector>

using NPyDecomposition = std::vector<std::vector<pybind11::array_t<float>>>;

/**
 * Convert python array to blaze N2D signal
 * @param data input python array
 * @return N2D signal
 */
drift::SignalN2D PyArrayToSignalN2D(const pybind11::array& data);

/**
 * Convert N2D signal to python array
 * @param data inptut N2D signal
 * @return
 */
pybind11::array_t<float> SignalN2DToPyArray(const drift::SignalN2D& data);

/**
 * Convert NWaveletDecomposition to Python array
 * @param decompositions
 * @return
 */
NPyDecomposition NWaveletDecompositionToNPy(
    const drift::NWaveletDecomposition& decompositions);

drift::NWaveletDecomposition NPyDecompositionToNW(
    const NPyDecomposition& decompositions);

pybind11::array_t<float> VecVecToPyArray(
    const blaze::DynamicVector<blaze::DynamicVector<float>>& data);

#endif  // SRC_DSP_BLAZE_UTILS_H_
