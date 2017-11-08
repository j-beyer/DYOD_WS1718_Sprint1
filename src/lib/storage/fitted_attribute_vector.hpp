#pragma once

#include "types.hpp"
#include "base_attribute_vector.hpp"

#include "vector"

namespace opossum {

// BaseAttributeVector is the abstract super class for all attribute vectors,
// e.g., FittedAttributeVector
template<typename T>
class FittedAttributeVector : private BaseAttributeVector {
 public:
  FittedAttributeVector() = default;
  virtual ~FittedAttributeVector() = default;

  // we need to explicitly set the move constructor to default when
  // we overwrite the copy constructor
  FittedAttributeVector(BaseAttributeVector&& other);
  FittedAttributeVector& operator=(BaseAttributeVector&& other);

  // returns the value at a given position
  virtual ValueID get(const size_t i) const;

  // inserts the value_id at a given position
  virtual void set(const size_t i, const ValueID value_id);

  // returns the number of values
  virtual size_t size() const;

  // returns the width of the values in bytes
  virtual AttributeVectorWidth width() const;

protected:
  std::vector<T> _data;
};
}  // namespace opossum
