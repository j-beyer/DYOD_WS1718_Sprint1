
#include "table_scan.hpp"

#include <set>
#include <utility>
#include <vector>

#include "resolve_type.hpp"
#include "storage/dictionary_column.hpp"
#include "storage/fitted_attribute_vector.hpp"
#include "storage/reference_column.hpp"
#include "storage/table.hpp"
#include "storage/value_column.hpp"

namespace opossum {

template <typename T>
std::shared_ptr<const Table> TableScan::TableScanImpl<T>::on_execute() {
  // create table
  // TODO(team): be careful with the chunk size; what happens if we use infinite chunk size here, but add a chunk later?
  auto result_table = std::make_shared<Table>();

  auto ref_col = std::dynamic_pointer_cast<ReferenceColumn>(_in_table->get_chunk(ChunkID{0}).get_column(_column_id));

  // auto deref_column_id = _column_id;
  auto deref_table = _in_table;
  auto is_reference = false;
  if (ref_col != nullptr) {
    is_reference = true;

    // deref_column_id = ref_col->referenced_column_id();
    deref_table = ref_col->referenced_table();
  }

  // create PosList and fill with values from scan
  _create_pos_list(is_reference);

  // create chunk (we will have a single chunk for the results)
  auto chunk = Chunk{};

  // Fill the chunk with reference columns to mirror the original relation
  for (auto col_id = ColumnID{0}; col_id < deref_table->col_count(); ++col_id) {
    // add column definition
    // TODO(team): is there a nicer way to combine add_column_definition and emplace_chunk ?
    result_table->add_column(deref_table->column_name(col_id), deref_table->column_type(col_id));

    // create reference columns
    auto reference_column = std::make_shared<ReferenceColumn>(deref_table, col_id, _pos_list);

    // add column to chunk
    chunk.add_column(reference_column);
  }

  // add chunk to table
  result_table->emplace_chunk(std::move(chunk));

  return result_table;
}

// This function implements the actual scanning
template <typename T>
void TableScan::TableScanImpl<T>::_create_pos_list(bool is_reference) {
  // first we need to know which data type we are targeting
  T search_value = type_cast<T>(_search_value);

  auto deref_column_id = _column_id;
  auto deref_table = _in_table;
  std::set<RowID> ref_pos_set;

  if (is_reference) {
    auto ref_col = std::dynamic_pointer_cast<ReferenceColumn>(_in_table->get_chunk(ChunkID{0}).get_column(_column_id));
    ref_pos_set = std::set<RowID>(ref_col->pos_list()->begin(), ref_col->pos_list()->end());
    deref_column_id = ref_col->referenced_column_id();
    deref_table = ref_col->referenced_table();
  }

  // we need to look at the correct column in each chunk, switching for the actual type of the column
  for (auto chunk_id = ChunkID{0}; chunk_id < deref_table->chunk_count(); ++chunk_id) {
    const auto& chunk = deref_table->get_chunk(chunk_id);
    const auto base_column = chunk.get_column(deref_column_id);

    auto value_column = std::dynamic_pointer_cast<ValueColumn<T>>(base_column);
    if (value_column != nullptr) {
      const auto& values = value_column->values();
      const auto chunk_offsets = _eval_operator(search_value, values, _get_comparator<T>());
      for (const auto chunk_offset : chunk_offsets) {
        auto cur_id = RowID{chunk_id, chunk_offset};
        // check whether we are running on a referenced column
        if (deref_column_id != _column_id || deref_table != _in_table) {
          if (ref_pos_set.find(cur_id) != ref_pos_set.end()) {
            _pos_list->push_back(cur_id);
          }
        } else {
          _pos_list->push_back(cur_id);
        }
      }
      continue;
    }

    auto dictionary_column = std::dynamic_pointer_cast<DictionaryColumn<T>>(base_column);
    if (dictionary_column != nullptr) {
      if (!_should_prune(search_value, dictionary_column)) {
        const auto chunk_offsets = _eval_operator(search_value, dictionary_column);
        for (const auto chunk_offset : chunk_offsets) {
          auto cur_id = RowID{chunk_id, chunk_offset};
          // check whether we are running on a referenced column
          if (deref_column_id != _column_id || deref_table != _in_table) {
            if (ref_pos_set.find(cur_id) != ref_pos_set.end()) {
              _pos_list->push_back(cur_id);
            }
          } else {
            _pos_list->push_back(cur_id);
          }
        }
      }
      continue;
    }

    // either the search value type does not match the column type,
    // or we are dealing with another column subclass that we do not know of yet
    throw std::runtime_error("Invalid column type or subclass!");
  }
}

template <typename T>
template <typename C>
Comparator<C> TableScan::TableScanImpl<T>::_get_comparator() const {
  switch (_scan_type) {
    case ScanType::OpEquals:
      return [](C c1, C c2) { return c1 == c2; };
    case ScanType::OpNotEquals:
      return [](C c1, C c2) { return c1 != c2; };
    case ScanType::OpLessThan:
      return [](C c1, C c2) { return c1 < c2; };
    case ScanType::OpLessThanEquals:
      return [](C c1, C c2) { return c1 <= c2; };
    case ScanType::OpGreaterThan:
      return [](C c1, C c2) { return c1 > c2; };
    case ScanType::OpGreaterThanEquals:
      return [](C c1, C c2) { return c1 >= c2; };
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

// TODO: naming of this function is confusing
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

  const auto compare_function = _get_comparator<ValueID>();

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

EXPLICITLY_INSTANTIATE_COLUMN_TYPES(TableScan::TableScanImpl);

}  // namespace opossum
