
#include "dictionary_column.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>

#include "fitted_attribute_vector.hpp"
#include "type_cast.hpp"
#include "utils/assert.hpp"
#include "value_column.hpp"

namespace opossum {

template <typename T>
DictionaryColumn<T>::DictionaryColumn(const std::shared_ptr<BaseColumn>& base_column) {
  auto val_column = std::dynamic_pointer_cast<ValueColumn<T>>(base_column);
  Assert(val_column != nullptr, "Compression is only supported for value columns!");

  const auto& values = val_column->values();
  Assert(!values.empty(), "Cannot compress empty value column!");

  _dictionary = std::make_shared<std::vector<T>>(values.cbegin(), values.cend());
  std::sort(_dictionary->begin(), _dictionary->end());
  auto last = std::unique(_dictionary->begin(), _dictionary->end());
  _dictionary->erase(last, _dictionary->end());

  const size_t attr_size = values.size();
  Assert(attr_size < std::numeric_limits<uint32_t>::max(), "Unsupported attribute vector width!");

  if (attr_size < std::numeric_limits<uint8_t>::max()) {
    _attribute_vector = std::make_shared<FittedAttributeVector<uint8_t>>(attr_size);
  } else if (attr_size < std::numeric_limits<uint16_t>::max()) {
    _attribute_vector = std::make_shared<FittedAttributeVector<uint16_t>>(attr_size);
  } else {
    _attribute_vector = std::make_shared<FittedAttributeVector<uint32_t>>(attr_size);
  }

  for (size_t attr_value_id = 0; attr_value_id < values.size(); ++attr_value_id) {
    const auto value = values[attr_value_id];
    const auto value_it = std::lower_bound(_dictionary->cbegin(), _dictionary->cend(), value);
    // we can assume that lower_bound will find a match,
    // as we inserted the values into the dictionary before
    const auto dict_value_id = value_it - _dictionary->cbegin();

    _attribute_vector->set(attr_value_id, ValueID(dict_value_id));
  }
}

template <typename T>
const AllTypeVariant DictionaryColumn<T>::operator[](const size_t i) const {
  return get(i);
}

template <typename T>
const T DictionaryColumn<T>::get(const size_t i) const {
  auto dict_id = _attribute_vector->get(i);
  DebugAssert(dict_id < _dictionary->size(), "Index out of bounds!");
  return (*_dictionary)[dict_id];
}

template <typename T>
void DictionaryColumn<T>::append(const AllTypeVariant&) {
  throw std::runtime_error("Appending to a compressed column is not supported!");
}

template <typename T>
std::shared_ptr<const std::vector<T>> DictionaryColumn<T>::dictionary() const {
  return _dictionary;
}

template <typename T>
std::shared_ptr<const BaseAttributeVector> DictionaryColumn<T>::attribute_vector() const {
  return _attribute_vector;
}

template <typename T>
const T& DictionaryColumn<T>::value_by_value_id(ValueID value_id) const {
  return _dictionary->at(value_id);
}

template <typename T>
ValueID DictionaryColumn<T>::lower_bound(T value) const {
  const auto lower_bound_it = std::lower_bound(_dictionary->cbegin(), _dictionary->cend(), value);
  if (lower_bound_it == _dictionary->cend()) {
    return INVALID_VALUE_ID;
  }
  const auto lower_bound_pos = lower_bound_it - _dictionary->cbegin();
  return static_cast<ValueID>(lower_bound_pos);
}

template <typename T>
ValueID DictionaryColumn<T>::lower_bound(const AllTypeVariant& value) const {
  T val = type_cast<T>(value);
  return lower_bound(val);
}

template <typename T>
ValueID DictionaryColumn<T>::upper_bound(T value) const {
  const auto upper_bound_id = std::upper_bound(_dictionary->cbegin(), _dictionary->cend(), value);
  if (upper_bound_id == _dictionary->cend()) {
    return INVALID_VALUE_ID;
  }
  const auto upper_bound_pos = upper_bound_id - _dictionary->cbegin();
  return static_cast<ValueID>(upper_bound_pos);
}

template <typename T>
ValueID DictionaryColumn<T>::upper_bound(const AllTypeVariant& value) const {
  T val = type_cast<T>(value);
  return upper_bound(val);
}

template <typename T>
size_t DictionaryColumn<T>::unique_values_count() const {
  return _dictionary->size();
}

template <typename T>
size_t DictionaryColumn<T>::size() const {
  return _attribute_vector->size();
}

EXPLICITLY_INSTANTIATE_COLUMN_TYPES(DictionaryColumn);

}  // namespace opossum
