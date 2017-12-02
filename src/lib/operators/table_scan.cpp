#include "table_scan.hpp"

#include "resolve_type.hpp"
#include "storage/dictionary_column.hpp"
#include "storage/fitted_attribute_vector.hpp"
#include "storage/reference_column.hpp"
#include "storage/table.hpp"
#include "storage/value_column.hpp"

namespace opossum {

TableScan::TableScan(const std::shared_ptr<const AbstractOperator> in, ColumnID column_id, const ScanType scan_type,
                     const AllTypeVariant search_value)
    : AbstractOperator(in), _column_id{column_id}, _scan_type{scan_type}, _search_value{search_value} {}

ColumnID TableScan::column_id() const { return _column_id; }

ScanType TableScan::scan_type() const { return _scan_type; }

const AllTypeVariant& TableScan::search_value() const { return _search_value; }

std::shared_ptr<const Table> TableScan::_on_execute() {
  const auto in_table = _input_table_left();
  const auto column_type = in_table->column_type(_column_id);

  auto _impl = make_unique_by_column_type<BaseTableScanImpl, TableScanImpl>(column_type, in_table, _column_id,
                                                                            _scan_type, _search_value);
  return _impl->on_execute();
}

template <typename T>
std::shared_ptr<const Table> TableScan::TableScanImpl<T>::on_execute() {
  // create table
  // TODO be careful with the chunk size; what happens if we use infinite chunk size here, but add a chunk later?
  auto result_table = std::make_shared<Table>();

  // create PosList and fill with values from scan
  _create_pos_list();

  // create chunk
  auto chunk = Chunk{};

  for (auto col_id = ColumnID{0}; col_id < _in_table->col_count(); ++col_id) {
    // add column definition
    // TODO(team): is there a nicer way to combine add_column_definition and emplace_chunk ?
    result_table->add_column(_in_table->column_name(col_id), _in_table->column_type(col_id));

    // create reference columns
    auto reference_column = std::make_shared<ReferenceColumn>(_in_table, col_id, _pos_list);

    // add column to chunk
    chunk.add_column(reference_column);
  }

  // add chunk to table
  result_table->emplace_chunk(std::move(chunk));

  return result_table;
}

template <typename T>
void TableScan::TableScanImpl<T>::_create_pos_list() {
  T search_value = type_cast<T>(_search_value);

  for (auto chunk_id = ChunkID{0}; chunk_id < _in_table->chunk_count(); ++chunk_id) {
    const auto& chunk = _in_table->get_chunk(chunk_id);
    const auto base_column = chunk.get_column(_column_id);

    auto value_column = std::dynamic_pointer_cast<ValueColumn<T>>(base_column);
    if (value_column != nullptr) {
      const auto& values = value_column->values();
      const auto chunk_offsets = _eval_operator(search_value, values, _get_comparator());
      for (const auto chunk_offset : chunk_offsets) {
        _pos_list->emplace_back(RowID{chunk_id, chunk_offset});
      }
      continue;
    }

    auto dictionary_column = std::dynamic_pointer_cast<DictionaryColumn<T>>(base_column);
    if (dictionary_column != nullptr) {
      if (!_should_prune(search_value, dictionary_column)) {
        const auto chunk_offsets = _eval_operator(search_value, dictionary_column);
        for (const auto chunk_offset : chunk_offsets) {
          _pos_list->emplace_back(RowID{chunk_id, chunk_offset});
        }
      }
      continue;
    }

    auto reference_column = std::dynamic_pointer_cast<ReferenceColumn>(base_column);
    if (reference_column != nullptr) {
      // TODO reuse pos_list?
      continue;
    }

    // either the search value type does not match the column type,
    // or we are dealing with another column subclass that we do not know of yet
    throw std::runtime_error("Invalid column type or subclass!");
  }
}
// TODO define alias for std::function<bool(const T&, const T&)> with using
template <typename T>
std::function<bool(const T&, const T&)> TableScan::TableScanImpl<T>::_get_comparator() const {
  switch (_scan_type) {
    case ScanType::OpEquals:
      return [](T t1, T t2) { return t1 == t2; };
    case ScanType::OpNotEquals:
      return [](T t1, T t2) { return t1 != t2; };
    case ScanType::OpLessThan:
      return [](T t1, T t2) { return t1 < t2; };
    case ScanType::OpLessThanEquals:
      return [](T t1, T t2) { return t1 <= t2; };
    case ScanType::OpGreaterThan:
      return [](T t1, T t2) { return t1 > t2; };
    case ScanType::OpGreaterThanEquals:
      return [](T t1, T t2) { return t1 >= t2; };
    default:
      throw std::runtime_error("Invalid scan type!");
  }
}

template <typename T>
std::function<bool(const ValueID, const ValueID)> TableScan::TableScanImpl<T>::_get_value_id_comparator() const {
  switch (_scan_type) {
    case ScanType::OpEquals:
      return [](ValueID t1, ValueID t2) { return t1 == t2; };
    case ScanType::OpNotEquals:
      return [](ValueID t1, ValueID t2) { return t1 != t2; };
    case ScanType::OpLessThan:
      return [](ValueID t1, ValueID t2) { return t1 < t2; };
    case ScanType::OpLessThanEquals:
      return [](ValueID t1, ValueID t2) { return t1 <= t2; };
    case ScanType::OpGreaterThan:
      return [](ValueID t1, ValueID t2) { return t1 > t2; };
    case ScanType::OpGreaterThanEquals:
      return [](ValueID t1, ValueID t2) { return t1 >= t2; };
    default:
      throw std::runtime_error("Invalid scan type!");
  }
}

template <typename T>
std::vector<ChunkOffset> TableScan::TableScanImpl<T>::_eval_operator(
    const T& search_value, const std::vector<T>& values,
    const std::function<bool(const T&, const T&)> compare_function) const {
  auto chunk_offsets = std::vector<ChunkOffset>{};
  // TODO can this be done as lambda?
  for (auto chunk_offset = ChunkOffset{0}; chunk_offset < values.size(); ++chunk_offset) {
    if (compare_function(values[chunk_offset], search_value)) {
      chunk_offsets.push_back(chunk_offset);
    }
  }
  return chunk_offsets;
}

template <typename T>
bool TableScan::TableScanImpl<T>::_should_prune(const T& search_value,
                                                const std::shared_ptr<DictionaryColumn<T>> dictionary_column) {
  const auto dictionary = dictionary_column->dictionary();
  const T& first = (*dictionary)[0];
  const T& last = (*dictionary)[dictionary->size() - 1];

  switch (_scan_type) {
    case ScanType::OpLessThan:
    case ScanType::OpLessThanEquals:
      if (first > search_value) {
        return true;
      }
      break;
    case ScanType::OpGreaterThan:
    case ScanType::OpGreaterThanEquals:
      if (last < search_value) {
        return true;
      }
      break;
    case ScanType::OpEquals:
      if (first > search_value || last < search_value) {
        return true;
      }
      break;
    case ScanType::OpNotEquals:
      if (first == search_value && last == search_value) {
        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

template <typename T>
std::vector<ChunkOffset> TableScan::TableScanImpl<T>::_eval_operator(
    const T& search_value, const std::shared_ptr<DictionaryColumn<T>> dictionary_column) const {
  auto chunk_offsets = std::vector<ChunkOffset>{};

  const auto lower_id = dictionary_column->lower_bound(search_value);
  const auto upper_id = dictionary_column->upper_bound(search_value);

  const auto contains_value = (lower_id != INVALID_VALUE_ID && lower_id != upper_id);

  const auto compare_function = _get_value_id_comparator();

  const auto attribute_vector = dictionary_column->attribute_vector();

  if (!contains_value) {
    switch (_scan_type) {
      case ScanType::OpNotEquals:
        for (auto position = ChunkOffset{0}; position < dictionary_column->size(); ++position) {
          chunk_offsets.push_back(position);
        }
        break;
      case ScanType::OpGreaterThan:
      case ScanType::OpGreaterThanEquals:
        for (auto position = ChunkOffset{0}; position < dictionary_column->size(); ++position) {
          if (attribute_vector->get(position) >= upper_id) {
            chunk_offsets.push_back(position);
          }
        }
        break;
      case ScanType::OpLessThan:
      case ScanType::OpLessThanEquals:
        for (auto position = ChunkOffset{0}; position < dictionary_column->size(); ++position) {
          if (attribute_vector->get(position) < lower_id) {
            chunk_offsets.push_back(position);
          }
        }
        break;
      case ScanType::OpEquals:
      default:
        break;
    }

  } else {
    for (auto position = ChunkOffset{0}; position < dictionary_column->size(); ++position) {
      if (compare_function(attribute_vector->get(position), lower_id)) {
        chunk_offsets.push_back(position);
      }
    }
  }

  return chunk_offsets;
}

}  // namespace opossum
