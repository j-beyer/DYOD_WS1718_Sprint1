﻿#include "value_column.hpp"

#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "type_cast.hpp"
#include "utils/assert.hpp"
#include "utils/performance_warning.hpp"

namespace opossum {

template <typename T>
const AllTypeVariant ValueColumn<T>::operator[](const size_t i) const {
  PerformanceWarning("operator[] used");

  // throws an exception if i is not there, slower than []
  return m_values.at(i);
}

template <typename T>
void ValueColumn<T>::append(const AllTypeVariant& val) {

  m_values.push_back(type_cast<T>(val));
}

template <typename T>
size_t ValueColumn<T>::size() const {
  // Implementation goes here
  return m_values.size();
}

EXPLICITLY_INSTANTIATE_COLUMN_TYPES(ValueColumn);

}  // namespace opossum
