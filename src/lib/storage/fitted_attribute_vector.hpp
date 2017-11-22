#pragma once

#include <limits>
#include <vector>

#include "base_attribute_vector.hpp"
#include "types.hpp"
#include "utils/assert.hpp"

namespace opossum {

// Implements an attribute vector with a specific (fitted) width, passed as template parameter
template <typename T>
class FittedAttributeVector : public BaseAttributeVector {
 public:
  explicit FittedAttributeVector(const size_t size) : BaseAttributeVector() {
    size_t max = std::numeric_limits<T>::max();
    Assert(size <= max,
           "Size " + std::to_string(size) + "too large for vector of width " + std::to_string(width()) + "!");

    // we resize the data vector here, so that we can freely insert values in the set() method
    // this assumes that filling the attribute vector with values from the value column is done
    // immediately after instantiation, so that the data vector is filled with correct values before use
    _data.resize(size);
  }

  ValueID get(const size_t i) const override {
    DebugAssert(i < _data.size(), "Index out of bounds!");
    return ValueID{_data[i]};
  }

  void set(const size_t i, const ValueID value_id) override {
    Assert(i <= size(), "Index " + std::to_string(i) + " too large for fitted attribute vector of size " +
                            std::to_string(size()) + "!");

    _data[i] = static_cast<T>(value_id);
  }

  size_t size() const override { return _data.size(); }

  AttributeVectorWidth width() const override { return sizeof(T); }

 protected:
  std::vector<T> _data;
};
}  // namespace opossum
