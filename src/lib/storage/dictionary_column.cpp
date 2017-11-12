
#include "dictionary_column.hpp"

#include <set>
#include <algorithm>

#include "value_column.hpp"
#include "fitted_attribute_vector.hpp"
#include "type_cast.hpp"

namespace opossum {

template<typename T>
DictionaryColumn<T>::DictionaryColumn(const std::shared_ptr<BaseColumn> &base_column)
{
    auto val_column = std::dynamic_pointer_cast<ValueColumn<T>>(base_column);

    if(val_column == nullptr){
        throw std::runtime_error("compression is only supported for value_columns");
    }

    // step 1 create dict vector
    _dictionary = std::make_shared<std::vector<T>>();
    _attribute_vector = std::dynamic_pointer_cast<BaseAttributeVector>(std::make_shared<FittedAttributeVector<uint64_t>>());

    std::set<T> distincts;

    const auto& values = val_column->values();

    for(const auto& value : values){
        distincts.insert(value);
    }

    _dictionary->reserve(distincts.size());

    for(const auto& unique : distincts){
        _dictionary->push_back(unique);
    }

    auto begin = _dictionary->begin();
    auto end = _dictionary->end();

    /* This loop may seem wonky but prevents multiple allocations in the _attribute_vector.
     * We have no way to tell the _attribute_vector its expected size, so this is the best we can do.
     * If everything goes well then the branch predictor will save us.
     */

    for(ValueID val_id {values.size() - 1}; val_id >= 0; --val_id){

        uint64_t dict_id = std::find(begin, end, values[static_cast<size_t>(val_id)]) - begin;

        _attribute_vector->set(static_cast<uint64_t>(val_id), static_cast<ValueID>(dict_id));
    }
}

template<typename T>
const AllTypeVariant DictionaryColumn<T>::operator[](const size_t i) const
{
    return get(i);
}

template<typename T>
const T DictionaryColumn<T>::get(const size_t i) const
{
    auto dict_id = _attribute_vector->get(i);
    return _dictionary->at(dict_id);
}

template<typename T>
void DictionaryColumn<T>::append(const AllTypeVariant &)
{
    throw std::runtime_error("Appending to a compressed column is not supported");
}

template<typename T>
std::shared_ptr<const std::vector<T>> DictionaryColumn<T>::dictionary() const
{
    return _dictionary;
}

template<typename T>
std::shared_ptr<const BaseAttributeVector> DictionaryColumn<T>::attribute_vector() const
{
    return _attribute_vector;
}

template<typename T>
const T &DictionaryColumn<T>::value_by_value_id(ValueID value_id) const
{
    return _dictionary->at(value_id);
}

template<typename T>
ValueID DictionaryColumn<T>::lower_bound(T value) const
{
    for(ValueID i{0}; i < _attribute_vector->size(); ++i){
        if(value_by_value_id(_attribute_vector->get(i)) >= value)
            return i;
    }

    return INVALID_VALUE_ID;
}

template<typename T>
ValueID DictionaryColumn<T>::lower_bound(const AllTypeVariant &value) const
{
    T val = type_cast<T>(value);
    return lower_bound(val);
}

template<typename T>
ValueID DictionaryColumn<T>::upper_bound(T value) const
{
    for(ValueID i{0}; i < _attribute_vector->size(); ++i){
        if(value_by_value_id(_attribute_vector->get(i)) > value)
            return i;
    }

    return INVALID_VALUE_ID;
}

template<typename T>
ValueID DictionaryColumn<T>::upper_bound(const AllTypeVariant &value) const
{
    T val = type_cast<T>(value);
    return upper_bound(val);
}

template<typename T>
size_t DictionaryColumn<T>::unique_values_count() const
{
    return _dictionary->size();
}

template<typename T>
size_t DictionaryColumn<T>::size() const
{
    return _attribute_vector->size();
}

EXPLICITLY_INSTANTIATE_COLUMN_TYPES(DictionaryColumn);

}  // namespace opossum
