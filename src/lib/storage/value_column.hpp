﻿#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base_column.hpp"

namespace opossum {

// ValueColumn is a specific column type that stores all its values in a vector
template <typename T>
class ValueColumn : public BaseColumn {
 public:
  // default constructor is auto generated

  // return the value at a certain position. If you want to write efficient operators, back off!
  const AllTypeVariant operator[](const size_t i) const override;

  // add a value to the end
  void append(const AllTypeVariant& val) override;

  // return the number of entries
  size_t size() const override;

  // Return all values. This is the preferred method to check a value at a certain index. Usually you need to
  // access more than a single value anyway.
  // e.g. auto& values = col.values(); and then: values.at(i); in your loop.
  const std::vector<T>& values() const;

 protected:
  std::vector<T> _values;
};

}  // namespace opossum
