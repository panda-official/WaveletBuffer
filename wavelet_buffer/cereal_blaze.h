#pragma once

#include <blaze/Blaze.h>

#include <cereal/cereal.hpp>

namespace cereal {

// ! Serialization of DynamicVector
template <class Archive, class T>
inline void CEREAL_SAVE_FUNCTION_NAME(Archive& ar,
                                      blaze::DynamicVector<T> const& vector) {
  ar(make_size_tag(
      static_cast<size_type>(vector.size())));  // number of elements
  for (auto&& v : vector) ar(v);
}

template <class Archive, class T>
inline void CEREAL_LOAD_FUNCTION_NAME(Archive& ar,
                                      blaze::DynamicVector<T>& vector) {
  size_type size;
  ar(make_size_tag(size));

  vector.resize(static_cast<std::size_t>(size));
  for (auto&& v : vector) ar(v);
}

// Serialization of CompressedMatrix
template <class Archive, class T>
inline void CEREAL_SAVE_FUNCTION_NAME(
    Archive& ar, blaze::CompressedMatrix<T> const& matrix) {
  ar(make_size_tag(static_cast<size_type>(matrix.rows())));
  ar(make_size_tag(static_cast<size_type>(matrix.columns())));
  ar(make_size_tag(
      static_cast<size_type>(matrix.nonZeros())));  // number of elements

  for (size_type i = 0; i < matrix.rows(); ++i) {
    size_type elements = matrix.nonZeros(i);
    ar(make_size_tag(elements));  // number of elements in row i

    for (auto e = matrix.begin(i); e != matrix.end(i); ++e) {
      ar(static_cast<size_type>(e->index()));
      ar(e->value());
    }
  }
}

template <class Archive, class T>
inline void CEREAL_LOAD_FUNCTION_NAME(Archive& ar,
                                      blaze::CompressedMatrix<T>& matrix) {
  size_type rows, columns;
  ar(make_size_tag(rows));
  ar(make_size_tag(columns));
  matrix.resize(rows, columns, false);

  size_type size;
  ar(make_size_tag(size));
  matrix.reserve(size);

  for (size_type i = 0; i < rows; ++i) {
    size_type elements;
    ar(make_size_tag(elements));
    for (size_type j = 0; j < elements; ++j) {
      size_type index;
      T value;
      ar(index, value);
      matrix.append(i, index, value);
    }
    matrix.finalize(i);
  }
}
}  // namespace cereal
