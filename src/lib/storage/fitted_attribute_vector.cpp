
#include <limits>
#include <string>

#include "fitted_attribute_vector.hpp"
#include "types.hpp"

namespace opossum {

template <typename T>
ValueID FittedAttributeVector<T>::get(const size_t i) const {
  return static_cast<ValueID>(_data.at(i));
}

template <typename T>
void FittedAttributeVector<T>::set(const size_t i, const ValueID value_id) {
  size_t max = std::numeric_limits<T>::max();
  if (i > max) {
    throw std::runtime_error("Index " + std::to_string(i) + " too large for fitted attribute vector size " +
                             std::to_string(width()));
  }

  // Be careful with this code this is off by one territory
  if (_data.size() <= i) {
    _data.resize(i + 1);
  }
  _data[i] = static_cast<T>(value_id);
}

template <typename T>
size_t FittedAttributeVector<T>::size() const {
  return _data.size();
}

template <typename T>
AttributeVectorWidth FittedAttributeVector<T>::width() const {
  return sizeof(T);
}

template class FittedAttributeVector<uint8_t>;
template class FittedAttributeVector<uint16_t>;
template class FittedAttributeVector<uint32_t>;

}  // namespace opossum
