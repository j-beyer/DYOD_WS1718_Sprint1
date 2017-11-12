#pragma once

#include "types.hpp"
#include "base_attribute_vector.hpp"

#include "vector"

namespace opossum {

// BaseAttributeVector is the abstract super class for all attribute vectors,
// e.g., FittedAttributeVector
template<typename T>
class FittedAttributeVector : public BaseAttributeVector {
 public:
  virtual ~FittedAttributeVector() = default;

  // returns the value at a given position
  virtual ValueID get(const size_t i) const override;

  // inserts the value_id at a given position
  virtual void set(const size_t i, const ValueID value_id) override;

  // returns the number of values
  virtual size_t size() const override;

  // returns the width of the values in bytes
  virtual AttributeVectorWidth width() const override;

protected:
  std::vector<T> _data;
};
}  // namespace opossum
