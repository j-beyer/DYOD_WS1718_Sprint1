
#include "dictionary_column.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <set>
#include <unordered_map>
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
  std::set<T> distincts(values.cbegin(), values.cend());
  std::unordered_map<T, ValueID> valueToDictIndex;

  _dictionary = std::make_shared<std::vector<T>>(distincts.cbegin(), distincts.cend());
  // as std::set is already sorting the distinct values for us, we can simply increase the index
  size_t index = 0;
  for (const auto& distinct_value : distincts) {
    valueToDictIndex[distinct_value] = index++;
  }

  Assert(!distincts.empty(), "Cannot compress empty value column!");

  // we can encode 2^8 = 256 distinct values in one byte
  size_t needed_width = std::ceil(std::log(distincts.size()) / std::log(256));

  switch (needed_width) {
    case 1:
      _attribute_vector = std::make_shared<FittedAttributeVector<uint8_t>>(val_column->size());
      break;
    case 2:
      _attribute_vector = std::make_shared<FittedAttributeVector<uint16_t>>(val_column->size());
      break;
    case 3:
    case 4:
      _attribute_vector = std::make_shared<FittedAttributeVector<uint32_t>>(val_column->size());
      break;
    default:
      throw std::runtime_error("Unsupported attribute vector width!");
  }

  for (size_t val_id = 0; val_id < values.size(); ++val_id) {
    _attribute_vector->set(val_id, valueToDictIndex[values[val_id]]);
  }
}

template <typename T>
const AllTypeVariant DictionaryColumn<T>::operator[](const size_t i) const {
  return get(i);
}

template <typename T>
const T DictionaryColumn<T>::get(const size_t i) const {
  auto dict_id = _attribute_vector->get(i);
  return _dictionary->at(dict_id);
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
  for (ValueID val_id{0}; val_id < _attribute_vector->size(); ++val_id) {
    if (value_by_value_id(_attribute_vector->get(val_id)) >= value) return val_id;
  }

  return INVALID_VALUE_ID;
}

template <typename T>
ValueID DictionaryColumn<T>::lower_bound(const AllTypeVariant& value) const {
  T val = type_cast<T>(value);
  return lower_bound(val);
}

template <typename T>
ValueID DictionaryColumn<T>::upper_bound(T value) const {
  for (ValueID val_id{0}; val_id < _attribute_vector->size(); ++val_id) {
    if (value_by_value_id(_attribute_vector->get(val_id)) > value) return val_id;
  }

  return INVALID_VALUE_ID;
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
