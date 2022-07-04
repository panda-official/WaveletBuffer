// Copyright 2020-2022 PANDA GmbH

#include "wavelet_buffer/wavelet.h"

namespace drift::wavelet {

blaze::CompressedMatrix<DataType> DaubechiesMat(size_t size, int order,
                                         Padding padding) {
  assert(order % 2 == 0);
  assert(size >= order);

  auto [Lo_D, Hi_D, Lo_R, Hi_R] = Orthfilt(dbwavf(order / 2));

  /* Reverse filters for convolution */
  std::reverse(Lo_D.begin(), Lo_D.end());
  std::reverse(Hi_D.begin(), Hi_D.end());

  /* Low filter part */
  auto mat = blaze::CompressedMatrix<DataType>(size, size);
  mat.reserve(size * Lo_D.size());
  if (padding == Padding::Periodized) {
    for (size_t i = 0; i < size / 2; ++i) {
      size_t ci = mat.columns() - 2 * i;
      if (ci > Lo_D.size()) {
        ci = 0;
      }
      for (size_t a = 0; a < Lo_D.size(); ++a) {
        if (ci >= Lo_D.size()) {
          ci = ci % Lo_D.size();
        }

        size_t j = i * 2 + ci;
        if (j >= mat.columns()) {
          j = j % mat.columns();
        }

        mat.append(i, j, Lo_D[ci]);
        ++ci;
      }
      mat.finalize(i);
    }

    /* Hi filter part */
    for (size_t i = 0; i < size / 2; ++i) {
      size_t ci = mat.columns() - 2 * i;
      if (ci > Hi_D.size()) {
        ci = 0;
      }
      for (size_t a = 0; a < Hi_D.size(); ++a) {
        if (ci >= Hi_D.size()) {
          ci = ci % Hi_D.size();
        }

        size_t j = i * 2 + ci;
        if (j >= mat.columns()) {
          j = j % mat.columns();
        }

        mat.append(size / 2 + i, j, Hi_D[ci]);
        ++ci;
      }
      mat.finalize(size / 2 + i);
    }
  } else if (padding == Padding::ZeroDerivative) {
    /* Calculate padding size */
    int paddingSize = Lo_D.size() - 1;

    /* Low filter part */
    for (size_t i = 0; i < size / 2; ++i) {
      int leftPadding = paddingSize / 2;
      int j0 = -leftPadding + 2 * static_cast<int>(i);

      /* Left padding */
      DataType lp = 0;
      for (int k = 0; k < -j0; ++k) {
        lp += Lo_D[k];
      }

      /* Right padding */
      DataType rp = 0;
      int l = j0 + Lo_D.size() - size;
      for (int k = 0; k < l; ++k) {
        rp += Lo_D[Lo_D.size() - 1 - k];
      }

      for (int k = 0; k < Lo_D.size(); ++k) {
        const int j = j0 + k;
        if (j == 0) {
          mat.append(i, j, lp + Lo_D[k]);
        } else if (j == size - 1) {
          mat.append(i, j, rp + Lo_D[k]);
        } else if (j > 0 && j < size - 1) {
          mat.append(i, j, Lo_D[k]);
        } else {
        }
      }
      mat.finalize(i);
    }

    /* Hi filter part */
    for (size_t i = 0; i < size / 2; ++i) {
      int leftPadding = paddingSize / 2;
      int j0 = -leftPadding + 2 * static_cast<int>(i);

      /* Left padding */
      DataType lp = 0;
      for (int k = 0; k < -j0; ++k) {
        lp += Hi_D[k];
      }

      /* Right padding */
      DataType rp = 0;
      int l = j0 + Hi_D.size() - size;
      for (int k = 0; k < l; ++k) {
        rp += Hi_D[Hi_D.size() - 1 - k];
      }

      for (int k = 0; k < Hi_D.size(); ++k) {
        const int j = j0 + k;
        if (j == 0) {
          mat.append(size / 2 + i, j, lp + Hi_D[k]);
        } else if (j == size - 1) {
          mat.append(size / 2 + i, j, rp + Hi_D[k]);
        } else if (j > 0 && j < size - 1) {
          mat.append(size / 2 + i, j, Hi_D[k]);
        } else {
        }
      }
      mat.finalize(size / 2 + i);
    }
  } else {
    return mat;
  }

  return mat;
}
std::tuple<Signal2D, Signal2D, Signal2D, Signal2D> dwt2(
    const Signal2D &x, const Signal2DCompressed &dmat_w,
    const Signal2DCompressed &dmat_h) {  // wrapper for dividing by subbands

  Signal2D r = dwt2s(x, dmat_w, dmat_h);

  size_t split_sz_w = dmat_w.columns() / 2;
  size_t split_sz_h = dmat_h.columns() / 2;

  Signal2D ll(split_sz_h, split_sz_w);
  Signal2D lh(split_sz_h, split_sz_w);
  Signal2D hl(split_sz_h, split_sz_w);
  Signal2D hh(split_sz_h, split_sz_w);
  ll = blaze::submatrix(r, 0, 0, split_sz_h, split_sz_w);
  lh = blaze::submatrix(r, split_sz_h, 0, split_sz_h, split_sz_w);
  hl = blaze::submatrix(r, 0, split_sz_w, split_sz_h, split_sz_w);
  hh = blaze::submatrix(r, split_sz_h, split_sz_w, split_sz_h, split_sz_w);

  return std::make_tuple(ll, lh, hl, hh);
}

Signal1D dbwavf(const int wnum) {
  assert(wnum <= 10);
  assert(wnum > 0);

  switch (wnum) {
    case 1:
      return {0.50000000000000, 0.50000000000000};
    case 2:
      return {0.34150635094622, 0.59150635094587, 0.15849364905378,
              -0.09150635094587};
    case 3:
      return {0.23523360389270,  0.57055845791731,  0.32518250026371,
              -0.09546720778426, -0.06041610415535, 0.02490874986589};
    case 4:
      return {0.16290171402562,  0.50547285754565,  0.44610006912319,
              -0.01978751311791, -0.13225358368437, 0.02180815023739,
              0.02325180053556,  -0.00749349466513};
    case 5:
      return {0.11320949129173, 0.42697177135271,  0.51216347213016,
              0.09788348067375, -0.17132835769133, -0.02280056594205,
              0.05485132932108, -0.00441340005433, -0.00889593505093,
              0.00235871396920};
    case 6:
      return {
          0.07887121600145072,   0.3497519070376178,   0.5311318799408691,
          0.22291566146501776,   -0.15999329944606142, -0.09175903203014758,
          0.0689440464873723,    0.019461604854164663, -0.022331874165094537,
          0.0003916255761485779, 0.003378031181463938, -0.0007617669028012533};
    case 7:
      return {0.05504971537285,  0.28039564181304,  0.51557424581833,
              0.33218624110566,  -0.10175691123173, -0.15841750564054,
              0.05042323250485,  0.05700172257986,  -0.02689122629486,
              -0.01171997078235, 0.00887489618962,  0.00030375749776,
              -0.00127395235906, 0.00025011342658};
    case 8:
      return {0.03847781105406,  0.22123362357624,  0.47774307521438,
              0.41390826621166,  -0.01119286766665, -0.20082931639111,
              0.00033409704628,  0.09103817842345,  -0.01228195052300,
              -0.03117510332533, 0.00988607964808,  0.00618442240954,
              -0.00344385962813, -0.00027700227421, 0.00047761485533,
              -0.00008306863060};
    case 9:
      return {0.02692517479416,  0.17241715192471,  0.42767453217028,
              0.46477285717278,  0.09418477475112,  -0.20737588089628,
              -0.06847677451090, 0.10503417113714,  0.02172633772990,
              -0.04782363205882, 0.00017744640673,  0.01581208292614,
              -0.00333981011324, -0.00302748028715, 0.00130648364018,
              0.00016290733601,  -0.00017816487955, 0.00002782275679};
    case 10:
      return {0.01885857879640,  0.13306109139687,  0.37278753574266,
              0.48681405536610,  0.19881887088440,  -0.17666810089647,
              -0.13855493935993, 0.09006372426666,  0.06580149355070,
              -0.05048328559801, -0.02082962404385, 0.02348490704841,
              0.00255021848393,  -0.00758950116768, 0.00098666268244,
              0.00140884329496,  -0.00048497391996, -0.00008235450295,
              0.00006617718320,  -0.00000937920789};
    default:
      return {};
  }
}

std::tuple<Signal1D, Signal1D, Signal1D, Signal1D> Orthfilt(
    const Signal1D &W_in) {
  auto qmf = [](Signal1D const &x) {
    // Signal1D y(x.rbegin(), x.rend());
    Signal1D y(x.size());
    for (size_t i = 0; i < x.size(); ++i) y[i] = x[x.size() - 1 - i];

    auto isEven = [](int n) {
      if (n % 2 == 0)
        return true;
      else
        return false;
    };
    int first;
    if (isEven(y.size())) {
      first = 1;
    } else {
      first = 2;
    }
    for (int i = first; i < y.size(); i = i + 2) {
      y[i] = -y[i];
    }
    return y;
  };
  auto sqrt = [](Signal1D const &x) {
    // Signal1D out;
    // out.reserve(x.size());
    Signal1D out(x.size());
    for (int i = 0; i < x.size(); ++i) {
      // out.push_back(std::sqrt(2) * (x[i]));
      out[i] = std::sqrt(2) * (x[i]);
    }

    return out;
  };

  // typename Signal1D::value_type W_in_sum = std::accumulate(W_in.begin(),
  // W_in.end(), 0);
  DataType W_in_sum = 0;
  for (size_t i = 0; i < W_in.size(); ++i) W_in_sum += W_in[i];

  Signal1D Lo_R = sqrt(W_in);
  Signal1D Hi_R = qmf(Lo_R);
  // Signal1D Hi_D(Hi_R.rbegin(), Hi_R.rend());
  Signal1D Hi_D(Hi_R.size());
  for (size_t i = 0; i < Hi_R.size(); ++i) Hi_D[i] = Hi_R[Hi_R.size() - 1 - i];
  // Signal1D Lo_D(Lo_R.rbegin(), Lo_R.rend());
  Signal1D Lo_D(Lo_R.size());
  for (size_t i = 0; i < Lo_R.size(); ++i) Lo_D[i] = Lo_R[Lo_R.size() - 1 - i];

  return {Lo_D, Hi_D, Lo_R, Hi_R};
}
Signal2D dwt2s(const Signal2D &x, const Signal2DCompressed &dmat_w,
               const Signal2DCompressed
                   &dmat_h) {  // whole image transform, no dividing by subbands
  assert(dmat_w.columns() == dmat_w.rows());
  assert(dmat_h.columns() == dmat_h.rows());
  assert(dmat_w.rows() == x.columns());
  assert(dmat_h.rows() == x.rows());

  Signal2D intermediate(x.rows(), x.columns());
  Signal2D out(x.rows(), x.columns());

  for (size_t row_idx = 0; row_idx < x.rows(); ++row_idx) {  // split by rows
    blaze::DynamicVector<DataType, blaze::rowVector> curr_row =
        blaze::row(x, row_idx);
    blaze::DynamicVector<DataType> row_split = dmat_w * blaze::trans(curr_row);
    blaze::row(intermediate, row_idx) = blaze::trans(row_split);
  }

  for (size_t col_idx = 0; col_idx < x.columns();
       ++col_idx) {  // split by columns
    blaze::DynamicVector<DataType> curr_col =
        blaze::column(intermediate, col_idx);
    blaze::DynamicVector<DataType> col_split = dmat_h * curr_col;
    blaze::column(out, col_idx) = col_split;
  }

  return out;
}

Signal2D idwt2(const Signal2D &ll, const Signal2D &lh, const Signal2D &hl,
               const Signal2D &hh, const Signal2DCompressed &dmat_w,
               const Signal2DCompressed &dmat_h) {
  assert(ll.rows() == lh.rows());
  assert(ll.rows() == hl.rows());
  assert(ll.rows() == hh.rows());
  assert(ll.columns() == lh.columns());
  assert(ll.columns() == hl.columns());
  assert(ll.columns() == hh.columns());
  assert(dmat_w.rows() == ll.columns() * 2);
  assert(dmat_h.rows() == ll.rows() * 2);

  Signal2D out(dmat_h.rows(), dmat_w.rows());
  blaze::submatrix(out, 0, 0, ll.rows(), ll.columns()) = ll;
  blaze::submatrix(out, ll.rows(), 0, lh.rows(), lh.columns()) = lh;
  blaze::submatrix(out, 0, ll.columns(), hl.rows(), hl.columns()) = hl;
  blaze::submatrix(out, ll.rows(), ll.columns(), hh.rows(), hh.columns()) = hh;

  return dwt2s(out, dmat_w, dmat_h);
}

std::tuple<blaze::DynamicVector<DataType>, blaze::DynamicVector<DataType>> dwt(
    const blaze::DynamicVector<DataType> &signal,
    const blaze::CompressedMatrix<DataType> &dmat) {
  assert(signal.size() >= dmat.columns());

  blaze::DynamicVector<DataType> low_subband(signal.size() / 2);
  blaze::DynamicVector<DataType> high_subband(signal.size() / 2);

  /* Raw convolution */
  if (dmat.rows() == 2) {
    low_subband = 0;
    high_subband = 0;
    for (size_t i = 0; i < signal.size() / 2; ++i) {
      for (size_t j = 0; j < dmat.columns(); ++j) {
        size_t index = i * 2 + j;
        /* Periodic padding */
        if (index >= signal.size()) {
          index -= signal.size();
        }
        low_subband[i] += signal[index] * dmat(0, j);
        high_subband[i] += signal[index] * dmat(1, j);
      }
    }

  } else if (signal.size() == dmat.columns()) {
    /* Convolve whole signal with two filters */
    blaze::DynamicVector<DataType> encoded = dmat * signal;
    const size_t split_index = signal.size() / 2;
    low_subband = blaze::subvector(encoded, 0, split_index);
    high_subband = blaze::subvector(encoded, split_index, split_index);

  } else {
    return {{}, {}};
  }

  return {low_subband, high_subband};
}

blaze::DynamicVector<DataType> idwt(
    const blaze::DynamicVector<DataType> &low_subband,
    const blaze::DynamicVector<DataType> &high_subband,
    const blaze::CompressedMatrix<DataType> &dmat) {
  assert(low_subband.size() == high_subband.size());

  blaze::DynamicVector<DataType> decoded(low_subband.size() * 2);
  decoded = 0;
  /* Raw convolution */
  if (dmat.rows() == 2) {
    size_t padding_size = dmat.columns() - 2;
    /* Start near end of signal for periodic padding */
    size_t i0 = low_subband.size() - padding_size / 2;
    for (size_t i = 0; i < low_subband.size() * 2; ++i) {
      size_t j0 = 0;
      /* Using odd/even filter elements to emulate convolution with added zeros
       between signal elements */
      if (i % 2 == 0) {
        j0 = 1;
      }
      for (size_t j = 0; j < dmat.columns(); j += 2) {
        size_t k = i0 + i / 2 + j / 2;
        if (k >= low_subband.size()) {
          k -= low_subband.size();
        }
        decoded[i] += dmat(0, j0 + j) * low_subband[k];
        decoded[i] += dmat(1, j0 + j) * high_subband[k];
      }
    }
  } else {
    blaze::DynamicVector<DataType> encoded(low_subband.size() +
        high_subband.size());
    blaze::subvector(encoded, 0, low_subband.size()) = low_subband;
    blaze::subvector(encoded, low_subband.size(), high_subband.size()) =
        high_subband;

    decoded = dmat * encoded;
  }

  return decoded;
}
}  // namespace drift::wavelet
