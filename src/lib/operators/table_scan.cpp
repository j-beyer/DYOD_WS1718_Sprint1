#include "table_scan.hpp"

#include "resolve_type.hpp"
#include "storage/dictionary_column.hpp"
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
  // add column definition
  result_table->add_column_definition(_in_table->column_name(_column_id), _in_table->column_type(_column_id));

  // create PosList and fill with values from scan
  _create_pos_list();

  // create reference column
  auto reference_column = std::make_shared<ReferenceColumn>(_in_table, _column_id, _pos_list);

  // create chunk
  auto chunk = Chunk{};

  // add column to chunk
  chunk.add_column(reference_column);

  // add chunk to table
  result_table->emplace_chunk(std::move(chunk));

  return result_table;
}

template <typename T>
void TableScan::TableScanImpl<T>::_create_pos_list() {
  const T search_value = type_cast<T>(_search_value);

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
    }

    auto dictionary_column = std::dynamic_pointer_cast<DictionaryColumn<T>>(base_column);
    if (dictionary_column != nullptr) {
      //      if (!_should_prune(search_value, dictionary_column)) {

      //      }
    }

    auto reference_column = std::dynamic_pointer_cast<ReferenceColumn>(base_column);
    if (reference_column != nullptr) {
      // TODO reuse pos_list?
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
std::vector<ChunkOffset> TableScan::TableScanImpl<T>::_eval_operator(
    const T& search_value, const std::vector<T>& values,
    const std::function<bool(const T&, const T&)> compare_function) const {
  auto chunk_offsets = std::vector<ChunkOffset>{};
  // TODO can this be done as lambda?
  for (auto chunk_offset = ChunkOffset{0}; chunk_offset < values.size(); ++chunk_offset) {
    if (compare_function(search_value, values[chunk_offset])) {
      chunk_offsets.push_back(chunk_offset);
    }
  }
  return chunk_offsets;
}

}  // namespace opossum
