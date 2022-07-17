// Copyright 2021-2022 PANDA GmbH

#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <tuple>
#include <utility>
#include <vector>

#include "wavelet_buffer/wavelet.h"
#include "wavelet_buffer/wavelet_buffer.h"

namespace drift {

namespace internal {

/**
 * build-in cache
 */

class WaveletMatrixCache {
 public:
  using WaveletMatrixStack =
      std::vector<std::vector<blaze::CompressedMatrix<DataType>>>;

  using WaveletMatrixStackCache =
      std::map<WaveletParameters, WaveletMatrixStack>;

 public:
  /**
   * Lookup for Daubechies matrices in cache and insert new one if not found
   * @param padded_shape
   * @param parameters
   * @return matrices
   */
  const WaveletMatrixStack &GenerateMatrices(
      const SignalShape &padded_shape, const WaveletParameters &parameters) {
    std::lock_guard lock(mutex_);

    auto it = cache_.find(parameters);
    if (it != cache_.end()) {
      return it->second;
    }

    WaveletMatrixStack stack(parameters.decomposition_steps);
    for (int step = 0; step < parameters.decomposition_steps; ++step) {
      stack[step].resize(padded_shape.size());
      const size_t wavelet_degree = parameters.wavelet_type * 2;
      for (int dim_index = 0; dim_index < padded_shape.size(); dim_index++) {
        const auto matrix_size =
            CalculateDecompositionSizeForStep(padded_shape[dim_index], step);
        stack[step][dim_index] =
            wavelet::DaubechiesMat(matrix_size, wavelet_degree);
      }
    }

    auto [inserted_it, inserted] = cache_.insert({parameters, stack});
    return inserted_it->second;
  }

  /**
   * Lookup for Daubechies matrices in cache and insert new one if not found
   * @param padded_shape
   * @param parameters
   * @return transposed matrices
   */
  const WaveletMatrixStack &GenerateTransMatrices(
      const SignalShape &padded_shape, const WaveletParameters &parameters) {
    std::lock_guard lock(trans_mutex_);

    auto it = trans_cache_.find(parameters);
    if (it != trans_cache_.end()) {
      return it->second;
    }

    // create copy
    WaveletMatrixStack stack = GenerateMatrices(padded_shape, parameters);
    for (auto &dim_stack : stack) {
      for (auto &matrix : dim_stack) {
        blaze::transpose(matrix);
      }
    }

    auto [inserted_it, inserted] = trans_cache_.insert({parameters, stack});
    return inserted_it->second;
  }

 private:
  /**
   * @brief Calculate the Decomposition size for a specific step
   *
   * @param padded_size size of the input for the step
   * @param step  the step
   * @return MatrixSize the size for decomposition
   */
  [[nodiscard]] static size_t CalculateDecompositionSizeForStep(
      size_t padded_size, int step) {
    return static_cast<size_t>(padded_size / std::pow(2, step));
  }

  WaveletMatrixStackCache cache_;
  WaveletMatrixStackCache trans_cache_;
  std::mutex mutex_;
  std::mutex trans_mutex_;
};

static WaveletMatrixCache matrix_cache;

/**
 * @brief Calculate the maximum possible decomposition steps depending on the
 * wavelet type and signal size
 *
 * Decomposition stops when all coefficients in the output corrupted by edge
 * effects
 *
 * @return maximum decomposition steps
 */
int CalculateMaxDecompositionSteps(WaveletTypes wavelet_type,
                                   const std::vector<size_t> &signal_shape) {
  auto shortest_dimension = [](auto &signal_shape) {
    return *std::min_element(std::begin(signal_shape), std::end(signal_shape));
  };

  const int type = static_cast<int>(wavelet_type);
  const size_t min_side = shortest_dimension(signal_shape);

  return std::floor(std::log2(min_side / (static_cast<double>(2 * type) - 1)));
}

/**
 * Get size of padding for each dimension in signal shape
 * @return
 */
SignalShape CalcPaddedSize(WaveletTypes wavelet_type,
                           const SignalShape &signal_shape,
                           const int decomposition_steps) {
  if (wavelet_type != WaveletTypes::kNone) {
    int actual_steps =
        CalculateMaxDecompositionSteps(wavelet_type, signal_shape);
    if (decomposition_steps >= 0) {
      assert(decomposition_steps <= actual_steps);
      actual_steps = decomposition_steps;
    }

    const size_t divider = std::pow(2, actual_steps);
    SignalShape padded_shape(signal_shape.size());
    for (int i = 0; i < signal_shape.size(); i++) {
      padded_shape[i] = divider * ((signal_shape[i] - 1) / divider + 1);
    }

    return padded_shape;
  } else {
    return signal_shape;
  }
}

/**
 * Apply wavelet transformation once on 2D signal
 * @param dest - destination subband iterator
 * @param denoiser
 * @param wavelet_matrix_w
 * @param wavelet_matrix_h
 * @param signal
 */
static void CalculateOneSideStep2D(
    WaveletDecomposition::Iterator dest,
    const DenoiseAlgorithm<DataType> &denoiser,
    const std::vector<blaze::CompressedMatrix<DataType>> &wavelet_matrix,
    Signal2D *signal, const size_t step = 0) {
  auto [cA, cH, cV, cD] =
      wavelet::dwt2(*signal, wavelet_matrix[0], wavelet_matrix[1]);

  *(dest + 0) = denoiser.Denoise(cH, step);
  *(dest + 1) = denoiser.Denoise(cV, step);
  *(dest + 2) = denoiser.Denoise(cD, step);

  std::swap(*signal, cA);
}

/**
 * Apply wavelet transformation once on 1D signal
 * @param dest
 * @param denoiser
 * @param wavelet_matrix
 * @param signal
 */
static void CalculateOneSideStep1D(
    WaveletDecomposition::Iterator dest,
    const DenoiseAlgorithm<DataType> &denoiser,
    const std::vector<blaze::CompressedMatrix<DataType>> &wavelet_matrix,
    Signal2D *signal, const size_t step = 0) {
  auto [low_subband, high_subband] =
      drift::wavelet::dwt(blaze::column(*signal, 0), wavelet_matrix[0]);

  // copy vector to subband matrix
  Signal2D data(high_subband.size(), 1);
  blaze::column(data, 0) = denoiser.Denoise(high_subband, step);
  *(dest + 0) = data;

  signal->resize(low_subband.size(), 1, true);
  blaze::column(*signal, 0) = low_subband;
}

/**
 * Facade method for different decomposition methods (1d, 2d..)
 * @param dest
 * @param denoiser
 * @param wavelet_matrix
 * @param signal
 */
static void CalculateOneSideStep(
    int dimension, WaveletDecomposition::Iterator dest,
    const DenoiseAlgorithm<DataType> &denoiser,
    const std::vector<blaze::CompressedMatrix<DataType>> &wavelet_matrix,
    Signal2D *signal, const size_t step = 0) {
  if (dimension == 1) {
    CalculateOneSideStep1D(dest, denoiser, wavelet_matrix, signal, step);
  } else {
    CalculateOneSideStep2D(dest, denoiser, wavelet_matrix, signal, step);
  }
}

/**
 * Decompose Nx2D signal
 * @param data
 * @param denoiser
 */
bool DecomposeImpl(const WaveletParameters &parameters,
                   NWaveletDecomposition *decomposition, const SignalN2D &data,
                   const DenoiseAlgorithm<DataType> &denoiser,
                   size_t start_signal, size_t signal_count) {
  /* Check shape for 2D */
  if (parameters.dimension() == 2 &&
      (data.size() != signal_count ||
       data[0].columns() != parameters.signal_shape[0] ||
       data[0].rows() != parameters.signal_shape[1])) {
    std::cerr << "Invalid 2D signal form";
    return false;
  }

  /* Check shape  for 1D */
  if (parameters.dimension() == 1 &&
      data[0].rows() != parameters.signal_shape[0]) {
    std::cerr << "Invalid 1D signal shape" << std::endl;
    return false;
  }

  const int subbands_per_wt = SubbandsPerWaveletTransform(parameters);
  const auto padded_size =
      CalcPaddedSize(parameters.wavelet_type, parameters.signal_shape,
                     parameters.decomposition_steps);

  /* Get convolution matrix stack if needed */
  std::vector<std::vector<blaze::CompressedMatrix<DataType>>>
      wavelet_matrix_stack;
  if (parameters.dimension() == 2) {
    wavelet_matrix_stack =
        matrix_cache.GenerateMatrices(padded_size, parameters);
  }

  /* Put decompose vectors for 1D */
  blaze::DynamicVector<DataType> scale_filter;
  if (parameters.wavelet_type != kNone) {
    scale_filter = wavelet::dbwavf(parameters.wavelet_type);
  }
  const auto [lo_d, hi_d, lo_r, hi_r] = wavelet::Orthfilt(scale_filter);
  blaze::CompressedMatrix<DataType> dmat(2, lo_d.size());
  blaze::row(dmat, 0) = blaze::trans(blaze::reverse(lo_d));
  blaze::row(dmat, 1) = blaze::trans(blaze::reverse(hi_d));

  for (int ch = start_signal; ch < start_signal + signal_count; ++ch) {
    auto channel = AddPadding(data[ch - start_signal], padded_size);

    for (int step = 0; step < parameters.decomposition_steps; ++step) {
      if (parameters.dimension() == 1) {
        CalculateOneSideStep(
            parameters.dimension(),
            (*decomposition)[ch].begin() + step * subbands_per_wt, denoiser,
            {dmat}, &channel, step);
      } else {
        CalculateOneSideStep(
            parameters.dimension(),
            (*decomposition)[ch].begin() + step * subbands_per_wt, denoiser,
            wavelet_matrix_stack[step], &channel, step);
      }
    }
    (*decomposition)[ch][parameters.decomposition_steps * subbands_per_wt] =
        channel;
  }

  return true;
}

/**
 *
 * @param steps
 * @param denoiser
 */
void DecomposeImpl(WaveletParameters params,
                   NWaveletDecomposition *decomposition, int steps,
                   const DenoiseAlgorithm<DataType> &denoiser) {
  // setup the calculation matrices
  auto wavelet_matrix_stack = matrix_cache.GenerateMatrices(
      CalcPaddedSize(params.wavelet_type, params.signal_shape,
                     params.decomposition_steps),
      params);

  const int subbands_per_wt = SubbandsPerWaveletTransform(params);

  auto old_step_count = params.decomposition_steps - steps;

  for (int channel = 0; channel < params.signal_number; ++channel) {
    // grab last subband from original and continue the decomposition
    Signal2D remainder =
        (*decomposition)[channel][old_step_count * subbands_per_wt];

    for (int additional_step = 0; additional_step < steps; ++additional_step) {
      const int step = additional_step + old_step_count;
      CalculateOneSideStep(
          params.dimension(),
          (*decomposition)[channel].begin() + step * subbands_per_wt, denoiser,
          wavelet_matrix_stack[step], &remainder, step);
    }
    (*decomposition)[channel][params.decomposition_steps * subbands_per_wt] =
        remainder;
  }
}

/**
 * Single composition step
 * @param low
 * @param src
 * @param wavelet_mat
 * @return
 */
blaze::DynamicMatrix<DataType> ComposeStep(
    int dimension, const blaze::DynamicMatrix<DataType> &low,
    typename WaveletDecomposition::ConstIterator src,
    const std::vector<blaze::CompressedMatrix<DataType>> &wavelet_mat) {
  if (dimension == 1) {
    const auto high = static_cast<blaze::DynamicMatrix<DataType>>(*(src - 1));
    auto result = drift::wavelet::idwt(blaze::column(low, 0),
                                       blaze::column(high, 0), wavelet_mat[0]);
    blaze::DynamicMatrix<DataType> data(result.size(), 1);
    blaze::column(data, 0) = result;
    return data;
  }

  return wavelet::idwt2(low,
                        static_cast<blaze::DynamicMatrix<DataType>>(*(src - 3)),
                        static_cast<blaze::DynamicMatrix<DataType>>(*(src - 2)),
                        static_cast<blaze::DynamicMatrix<DataType>>(*(src - 1)),
                        wavelet_mat[0], wavelet_mat[1]);
}

NWaveletDecomposition ComposeImpl(const WaveletParameters &params,
                                  const NWaveletDecomposition &decomposition,
                                  size_t steps, size_t start_channel,
                                  size_t count) {
  NWaveletDecomposition subbands(count);

  /* Get convolution matrix stack if needed */
  std::vector<std::vector<blaze::CompressedMatrix<DataType>>>
      wavelet_matrix_stack;
  if (params.dimension() == 2) {
    wavelet_matrix_stack = matrix_cache.GenerateTransMatrices(
        CalcPaddedSize(params.wavelet_type, params.signal_shape,
                       params.decomposition_steps),
        params);
  }

  /* Put compose vectors for 1D */
  blaze::DynamicVector<DataType> scale_filter;
  if (params.wavelet_type != kNone) {
    scale_filter = wavelet::dbwavf(params.wavelet_type);
  }
  const auto [lo_d, hi_d, lo_r, hi_r] = wavelet::Orthfilt(scale_filter);
  blaze::CompressedMatrix<DataType> dmat(2, lo_d.size());
  blaze::row(dmat, 0) = blaze::trans(blaze::reverse(lo_r));
  blaze::row(dmat, 1) = blaze::trans(blaze::reverse(hi_r));

  const auto subbands_per_wt = internal::SubbandsPerWaveletTransform(params);
  for (int ch = start_channel; ch < start_channel + count; ++ch) {
    /* Convert sparse matrix with image to dense */
    size_t sub_index = ch - start_channel;
    subbands[sub_index] = decomposition[ch];
    auto channel = static_cast<blaze::DynamicMatrix<DataType>>(
        decomposition[ch][params.decomposition_steps * subbands_per_wt]);

    for (int i = params.decomposition_steps; i > steps; --i) {
      if (params.dimension() == 1) {
        channel = ComposeStep(params.dimension(), channel,
                              decomposition[ch].begin() + i * subbands_per_wt,
                              {dmat});
      } else {
        channel = ComposeStep(params.dimension(), channel,
                              decomposition[ch].begin() + i * subbands_per_wt,
                              wavelet_matrix_stack[i - 1]);
      }
    }

    subbands[sub_index].resize(steps * subbands_per_wt + 1, true);
    subbands[sub_index][subbands[sub_index].size() - 1] = channel;
  }

  return subbands;
}

bool ComposeImpl(const WaveletParameters &params, SignalN2D *data,
                 const NWaveletDecomposition &decomposition, size_t steps,
                 size_t start_signal, size_t count) {
  *data = SignalN2D(count, blaze::DynamicMatrix<DataType>());

  auto subbands =
      internal::ComposeImpl(params, decomposition, steps, start_signal, count);

  // crop padding
  SignalShape scaled_shape(params.signal_shape.size());
  std::transform(params.signal_shape.begin(), params.signal_shape.end(),
                 scaled_shape.begin(),
                 [&steps](auto x) { return x / std::pow(2, steps); });

  const auto factor =
      std::pow(params.dimension() == 2 ? 2 : std::sqrt(2), steps);
  for (int ch = 0; ch < count; ++ch) {
    auto aprox = &subbands[ch][subbands[ch].size() - 1];
    internal::CropPadding(aprox, scaled_shape);
    if (steps > 0) {
      *aprox /= factor;
    }
    (*data)[ch] = *aprox;
  }

  return true;
}

int SubbandsPerWaveletTransform(const WaveletParameters &parameters) {
  return (parameters.dimension() == 1) ? 1 : 3;
}
}  // namespace internal

DataType Distance(const WaveletBuffer &lhs, const WaveletBuffer &rhs) {
  const auto &lhs_par = lhs.parameters();
  const auto &rhs_par = rhs.parameters();
  assert(lhs_par.signal_number == rhs_par.signal_number &&
         lhs_par.signal_shape == rhs_par.signal_shape &&
         "Signals must have the same form");

  static_assert(std::numeric_limits<DataType>::has_signaling_NaN);

  auto CalcDistance = [](const NWaveletDecomposition &a_decs,
                         const NWaveletDecomposition &b_decs) {
    double result = 0;
    for (int ch = 0; ch < a_decs.size(); ++ch) {
      for (int i = 0; i < a_decs[ch].size(); ++i) {
        auto &a = a_decs[ch][i];
        auto &b = b_decs[ch][i];
        result += std::pow(blaze::norm(b - a), 2);
      }
    }

    return result;
  };

  const auto &lhs_decs = lhs.decompositions();
  const auto &rhs_decs = rhs.decompositions();

  double ret;

  const auto step_diff =
      lhs_par.decomposition_steps - rhs_par.decomposition_steps;
  if (step_diff == 0) {
    ret = CalcDistance(lhs_decs, rhs_decs);
  } else {
    return std::numeric_limits<DataType>::quiet_NaN();
  }

  const size_t signal_size =
      std::accumulate(lhs_par.signal_shape.begin(), lhs_par.signal_shape.end(),
                      1, std::multiplies<>());
  const size_t elements_num = lhs_par.signal_number * signal_size;

  return ret / elements_num;
}

size_t DecompositionSize(const WaveletParameters &parameters) {
  const int subbands_per_wt = internal::SubbandsPerWaveletTransform(parameters);
  return parameters.decomposition_steps * subbands_per_wt + 1;
}

blaze::DynamicVector<blaze::DynamicVector<DataType>> EnergyDistribution(
    const WaveletBuffer &buffer) {
  using Distribution = blaze::DynamicVector<blaze::DynamicVector<DataType>>;
  if (buffer.IsEmpty()) {
    return {};
  }

  auto params = buffer.parameters();
  auto step_divisor = buffer.parameters().signal_shape.size() == 1 ? 1 : 3;
  SignalShape scaled_shape(params.signal_shape.size());

  Distribution dist(buffer.decompositions().size());
  for (int n = 0; n < buffer.decompositions().size(); ++n) {
    const auto &decomposition = buffer.decompositions()[n];
    blaze::DynamicVector<DataType> subband_dist(decomposition.size());

    for (int s = 0; s < decomposition.size(); ++s) {
      double energy;  // use double not collect too big error
      blaze::CompressedMatrix<double> subband = buffer.decompositions()[n][s];
      energy = blaze::sum(subband % subband);
      subband_dist[s] = static_cast<DataType>(energy);
    }

    dist[n] = subband_dist;
  }
  return dist;
}

}  // namespace drift
