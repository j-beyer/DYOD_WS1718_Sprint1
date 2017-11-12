
#include "types.hpp"
#include "fitted_attribute_vector.hpp"

namespace opossum {

template<typename T>
ValueID FittedAttributeVector<T>::get(const size_t i) const
{
    return static_cast<ValueID>(_data.at(i));
}

template<typename T>
void FittedAttributeVector<T>::set(const size_t i, const ValueID value_id)
{
    // Be careful with this code this is off by one territory
    if(_data.size() <= i){
        _data.resize(i+1);
    }
    _data[i] = static_cast<T>(value_id);
}

template<typename T>
size_t FittedAttributeVector<T>::size() const
{
    return _data.size();
}

template<typename T>
AttributeVectorWidth FittedAttributeVector<T>::width() const
{
    return sizeof(T);
}

template class FittedAttributeVector<uint64_t>;


}  // namespace opossum
