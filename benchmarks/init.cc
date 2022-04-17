// Copyright 2022 PANDA GmbH

#include "init.h"

namespace drift::utils {

drift::dsp::Signal1D GetRandomSignal(size_t length) {
  std::uniform_real_distribution<drift::dsp::Signal1D::ElementType>
      distribution;
  std::mt19937 random_engine;

  auto a = blaze::generate(length, [&random_engine, &distribution](size_t i) {
    return distribution(random_engine);
  });

  return a;
}

drift::dsp::SignalN2D GetRandomSignal(size_t rows, size_t columns) {
  std::uniform_real_distribution<drift::dsp::Signal2D::ElementType>
      distribution;
  std::mt19937 random_engine;

  auto a = blaze::generate<blaze::rowMajor>(
      rows, columns, [&random_engine, &distribution](size_t i, size_t j) {
        return distribution(random_engine);
      });

  return {a};
}
}  // namespace drift::utils
