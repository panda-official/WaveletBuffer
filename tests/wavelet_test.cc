// Copyright 2021-2022 PANDA GmbH

#include "wavelet_buffer/wavelet.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "wavelet_buffer/padding.h"

using drift::DataType;
using drift::Signal1D;
using drift::ZeroDerivativePaddingAlgorithm;
using drift::wavelet::DaubechiesMat;
using drift::wavelet::dbwavf;
using drift::wavelet::Orthfilt;

TEST_CASE("DaubechiesMat", "[wavelet]") {
  auto wnum = GENERATE(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
  const auto w = dbwavf(wnum);
  auto [Lo_D, Hi_D, Lo_R, Hi_R] = Orthfilt(w);
  std::reverse(Lo_D.begin(), Lo_D.end());
  std::reverse(Hi_D.begin(), Hi_D.end());

  const size_t signalLength = 24;
  const auto dmat = DaubechiesMat(signalLength, wnum * 2);

  const size_t fl = wnum * 2;
  REQUIRE(blaze::subvector(blaze::row(dmat, 0), 0, fl) == Lo_D);
  REQUIRE(blaze::subvector(blaze::row(dmat, dmat.rows() / 2), 0, fl) == Hi_D);

  const auto sumLow = blaze::sum(blaze::row(dmat, 0));
  for (size_t i = 0; i < dmat.rows() / 2; ++i) {
    REQUIRE(Catch::Approx(sumLow) == blaze::sum(blaze::row(dmat, i)));
  }

  const auto sumHigh = blaze::sum(blaze::row(dmat, dmat.rows() / 2));
  for (size_t i = dmat.rows() / 2; i < dmat.rows(); ++i) {
    /* Set scale = 1 due problem with Approx(0.0) comparation */
    REQUIRE(Catch::Approx(sumHigh).scale(1.) ==
            blaze::sum(blaze::row(dmat, i)));
  }
}

TEST_CASE("dbwavf", "[wavelet]") {
  std::array<std::vector<DataType>, 11> coeffs;
  coeffs[1] = {0.5000000000000000, 0.5000000000000000};
  coeffs[2] = {0.3415063509461097, 0.5915063509461097, 0.15849364905389035,
               -0.09150635094610966};
  coeffs[3] = {0.23523360389208187,  0.5705584579157218,
               0.3251825002631163,   -0.09546720778416369,
               -0.06041610415519811, 0.024908749868441868};
  coeffs[4] = {0.16290171402564918,   0.5054728575459145,   0.44610006912337985,
               -0.019787513117822324, -0.13225358368451987, 0.02180815023708863,
               0.02325180053549088,   -0.007493494665180737};
  coeffs[5] = {0.1132094912917792,    0.4269717713525142,
               0.5121634721295986,    0.09788348067390469,
               -0.17132835769146745,  -0.02280056594177365,
               0.05485132932106683,   -0.004413400054179128,
               -0.008895935050977096, 0.002358713969533936};
  coeffs[6] = {
      0.07887121600145072,   0.3497519070376178,   0.5311318799408691,
      0.22291566146501776,   -0.15999329944606142, -0.09175903203014758,
      0.0689440464873723,    0.019461604854164663, -0.022331874165094537,
      0.0003916255761485779, 0.003378031181463938, -0.0007617669028012533};
  coeffs[7] = {
      0.05504971537281186,    0.28039564181276255,   0.5155742458180986,
      0.3321862411055397,     -0.10175691123134625,  -0.15841750564033286,
      0.050423232504694084,   0.057001722579871586,  -0.02689122629484544,
      -0.01171997078210329,   0.008874896189680764,  0.00030375749770106936,
      -0.0012739523590936867, 0.00025011342656124536};
  coeffs[8] = {
      0.03847781105407624,   0.22123362357612492,     0.4777430752138737,
      0.4139082662111959,    -0.011192867666880218,   -0.20082931639048904,
      0.0003340970462201188, 0.09103817842365776,     -0.01228195052284841,
      -0.031175103325139432, 0.009886079648350761,    0.006184422409815923,
      -0.003443859628441809, -0.00027700227447938935, 0.00047761485564962614,
      -8.30686306866127e-05};
  coeffs[9] = {
      0.0269251747946628,     0.17241715190697796,     0.42767453217970763,
      0.46477285718314737,    0.09418477475318378,     -0.2073758809009385,
      -0.0684767745123831,    0.1050341711395062,      0.021726337730614546,
      -0.04782363206009703,   0.00017744640661651893,  0.015812082926255862,
      -0.0033398101131385774, -0.0030274802875450662,  0.0013064836402472458,
      0.00016290733567609223, -0.00017816487951077768, 2.7822757017154863e-05};
  coeffs[10] = {
      0.01885857879612069,   0.1330610913969209,      0.3727875357432334,
      0.48681405536682,      0.19881887088450872,     -0.1766681008970563,
      -0.13855493936048316,  0.09006372426669666,     0.06580149355053501,
      -0.050483285598389716, -0.020829624043800808,   0.023484907048698562,
      0.002550218483907239,  -0.00758950116792825,    0.0009866626824816026,
      0.0014088432950973382, -0.00048497391992820553, -8.235450304538899e-05,
      6.617718342555339e-05, -9.379207813750205e-06};

  const int wnum = GENERATE(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

  const auto r = dbwavf(wnum);
  REQUIRE(r.size() == coeffs[wnum].size());
  for (size_t i = 0; i < r.size(); ++i) {
    REQUIRE(Catch::Approx(r[i]) == coeffs[wnum][i]);
  }
}
