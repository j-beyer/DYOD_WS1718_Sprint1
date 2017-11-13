#pragma once

#include <limits>
#include <vector>

#include "base_attribute_vector.hpp"
#include "types.hpp"
#include "utils/assert.hpp"

namespace opossum {

// BaseAttributeVector is the abstract super class for all attribute vectors,
// e.g., FittedAttributeVector
template <typename T>
class FittedAttributeVector : public BaseAttributeVector {
 public:
  explicit FittedAttributeVector(const size_t size) : BaseAttributeVector() {
    size_t max = std::numeric_limits<T>::max();
    Assert(size <= max,
           "Size " + std::to_string(size) + "too large for vector of width " + std::to_string(width()) + "!");

    _data.resize(size);
  }

  // returns the value at a given position
  ValueID get(const size_t i) const override { return ValueID{_data.at(i)}; }

  // inserts the value_id at a given position
  void set(const size_t i, const ValueID value_id) override {
    Assert(i <= size(), "Index " + std::to_string(i) + " too large for fitted attribute vector of size " +
                            std::to_string(size()) + "!");

    _data[i] = static_cast<T>(value_id);
  }

  // returns the number of values
  size_t size() const override { return _data.size(); }

  // returns the width of the values in bytes
  AttributeVectorWidth width() const override { return sizeof(T); }

 protected:
  std::vector<T> _data;
};
}  // namespace opossum
