
#include "dictionary_column.hpp"

#include <set>
#include <algorithm>

#include "value_column.hpp"
#include "fitted_attribute_vector.hpp"
#include "type_cast.hpp"

namespace opossum {

template<typename T>
DictionaryColumn::DictionaryColumn(const std::shared_ptr<BaseColumn> &base_column)
{
    //auto val_column = std::dynamic_pointer_cast<std::shared_ptr<ValueColumn<T>>> base_column;

    // step 1 create dict vector
    _dictionary = std::make_shared<std::vector<T>>();
    _attribute_vector = std::make_shared<FittedAttributeVector<uint64_t>>();

    std::set<T> distincts;

    for(size_t val_id = 0; val_id < base_column->size(); ++val_id){
        distincts.insert(*base_column)[val_id];
    }
    std::copy(distincts.begin(), distincts.end(), std::back_inserter(*_dictionary));

    auto begin = distincts.begin();

    for(size_t val_id = 0; val_id < base_column->size(); ++val_id){
        uint64_t dict_id = distincts.find((*base_column)[val_id]) - begin;
        _attribute_vector->set(val_id, dict_id);
    }
}

template<typename T>
const AllTypeVariant DictionaryColumn::operator[](const size_t i) const
{
    return get(i);
}

template<typename T>
const T DictionaryColumn::get(const size_t i) const
{
    auto dict_id = _attribute_vector->get(i);
    return _dictionary->at(dict_id);
}

template<typename T>
void DictionaryColumn::append(const AllTypeVariant &)
{
    throw std::runtime_error("Appending to a compressed column is not supported");
}

template<typename T>
std::shared_ptr<const std::vector<T>> DictionaryColumn::dictionary() const
{
    return _dictionary;
}

template<typename T>
std::shared_ptr<const BaseAttributeVector> DictionaryColumn::attribute_vector() const
{
    return _attribute_vector;
}

template<typename T>
const T &DictionaryColumn::value_by_value_id(ValueID value_id) const
{
    return _dictionary->at(value_id);
}

template<typename T>
ValueID DictionaryColumn::lower_bound(T value) const
{
    for(ValueID i{0}; i < _attribute_vector->size; ++i){
        if(value_by_value_id(*_attribute_vector[i]) >= value)
            return ValueID;
    }

    return INVALID_VALUE_ID;
}

template<typename T>
ValueID DictionaryColumn::lower_bound(const AllTypeVariant &value) const
{
    T val = type_cast<T>(value);
    return lower_bound(val);
}

template<typename T>
ValueID DictionaryColumn::upper_bound(T value) const
{
    for(ValueID i{0}; i < _attribute_vector->size; ++i){
        if(value_by_value_id(*_attribute_vector[i]) > value)
            return ValueID;
    }

    return INVALID_VALUE_ID;
}

template<typename T>
ValueID DictionaryColumn::upper_bound(const AllTypeVariant &value) const
{
    T val = type_cast<T>(value);
    return upper_bound(val);
}

template<typename T>
size_t DictionaryColumn::unique_values_count() const
{
    _dictionary->size();
}

template<typename T>
size_t DictionaryColumn::size() const
{
    _attribute_vector->size();
}

EXPLICITLY_INSTANTIATE_COLUMN_TYPES(DictionaryColumn);

}  // namespace opossum
