#pragma once

#include "types.hpp"
#include "fitted_attribute_vector.hpp"

namespace opossum {


template<typename T>
FittedAttributeVector::FittedAttributeVector(BaseAttributeVector && other)
: _data{std::move(other._data)}
{
}

template<typename T>
FittedAttributeVector &FittedAttributeVector::operator=(BaseAttributeVector && other)
: _data{std::move(other._data)}
{
}

template<typename T>
ValueID FittedAttributeVector::get(const size_t i) const
{
    return _data.at(i);
}

template<typename T>
void FittedAttributeVector::set(const size_t i, const ValueID value_id)
{
    if(_data.size() < i){
        _data.resize(i+1);
    }
    data.at(i) = static_cast<T>(value_id);
}

template<typename T>
size_t FittedAttributeVector::size() const
{
    return _data.size();
}

template<typename T>
AttributeVectorWidth FittedAttributeVector::width() const
{
    return sizeof(T);
}


}  // namespace opossum
