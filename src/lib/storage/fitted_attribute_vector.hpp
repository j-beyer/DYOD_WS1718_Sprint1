#pragma once

#include "base_attribute_vector.hpp"
#include "types.hpp"

#include "vector"

namespace opossum {

// BaseAttributeVector is the abstract super class for all attribute vectors,
// e.g., FittedAttributeVector
template <typename T>
class FittedAttributeVector : public BaseAttributeVector {
 public:
  virtual ~FittedAttributeVector() = default;

  // returns the value at a given position
  ValueID get(const size_t i) const final;

  // inserts the value_id at a given position
  void set(const size_t i, const ValueID value_id) final;

  // returns the number of values
  size_t size() const final;

  // returns the width of the values in bytes
  AttributeVectorWidth width() const final;

 protected:
  std::vector<T> _data;
};
}  // namespace opossum
