// Copyright 2020-2022 PANDA GmbH

#include <wavelet_buffer/cereal_blaze.h>

#include <catch2/catch_test_macros.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

TEST_CASE("Serialize DynamicVector") {
  blaze::DynamicVector<double> vec{1, 2, 3, 4, 5};
  std::stringstream ss;
  {
    cereal::PortableBinaryOutputArchive oarchive(ss);
    oarchive(vec);
  }
  blaze::DynamicVector<double> vec2;
  {
    cereal::PortableBinaryInputArchive iarchive(ss);
    iarchive(vec2);
  }
  REQUIRE(vec == vec2);
}

TEST_CASE("Serialize DynamicVector<DynamicVector>") {
  blaze::DynamicVector<blaze::DynamicVector<double>> vec{{1, 2}, {3, 4, 5}};
  std::stringstream ss;
  {
    cereal::PortableBinaryOutputArchive oarchive(ss);
    oarchive(vec);
  }
  blaze::DynamicVector<blaze::DynamicVector<double>> vec2;
  {
    cereal::PortableBinaryInputArchive iarchive(ss);
    iarchive(vec2);
  }
  REQUIRE(vec == vec2);
}

TEST_CASE("Serialize DynamicMatrix") {
  blaze::DynamicMatrix<double> mat{{1, 0, 3}, {0, 0, 6}};
  std::stringstream ss;
  {
    cereal::PortableBinaryOutputArchive oarchive(ss);
    oarchive(mat);
  }
  blaze::DynamicMatrix<double> mat2;
  {
    cereal::PortableBinaryInputArchive iarchive(ss);
    iarchive(mat2);
  }
  REQUIRE(mat == mat2);
}

TEST_CASE("Serialize CompressedMatrix") {
  blaze::CompressedMatrix<double> mat{{1, 0, 3}, {0, 0, 6}};
  std::stringstream ss;
  {
    cereal::PortableBinaryOutputArchive oarchive(ss);
    oarchive(mat);
  }
  blaze::CompressedMatrix<double> mat2;
  {
    cereal::PortableBinaryInputArchive iarchive(ss);
    iarchive(mat2);
  }
  REQUIRE(mat == mat2);
}
