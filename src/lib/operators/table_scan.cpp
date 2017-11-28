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

  _output = _impl->on_execute();
  return _output;
}

template <typename T>
std::shared_ptr<const Table> TableScan::TableScanImpl<T>::on_execute() {
  // create table
  // TODO be careful with the chunk size; what happens if we use infinite chunk size here, but add a chunk later?
  auto result_table = std::make_shared<Table>();
  // add column definition
  result_table->add_column_definition(_in_table->column_name(_column_id), _in_table->column_type(_column_id));

  // create PosList and fill with values from scan
  auto pos_list = create_pos_list();

  // create reference column
  auto reference_column = std::make_shared<ReferenceColumn>(_in_table, _column_id, pos_list);

  // create chunk
  auto chunk = Chunk{};

  // add column to chunk
  chunk.add_column(reference_column);

  // add chunk to table
  result_table->emplace_chunk(std::move(chunk));

  return result_table;
}

template <typename T>
std::shared_ptr<PosList> TableScan::TableScanImpl<T>::create_pos_list() const {
  // create PosList
  auto pos_list = std::make_shared<PosList>();

  const T search_value = type_cast<T>(_search_value);

  for (auto chunk_id = ChunkID{0}; chunk_id < _in_table->chunk_count(); ++chunk_id) {
    const auto& chunk = _in_table->get_chunk(chunk_id);
    const auto base_column = chunk.get_column(_column_id);

    auto value_column = std::dynamic_pointer_cast<ValueColumn<T>>(base_column);
    if (value_column != nullptr) {
      const auto& values = value_column->values();
    }

    auto dictionary_column = std::dynamic_pointer_cast<DictionaryColumn<T>>(base_column);
    if (dictionary_column != nullptr) {
    }

    auto reference_column = std::dynamic_pointer_cast<ReferenceColumn>(base_column);
    if (reference_column != nullptr) {
      // TODO reuse pos_list?
    }

    // either the search value type does not match the column type,
    // or we are dealing with another column subclass that we do not know of yet
    throw std::runtime_error("Invalid column type or subclass!");
  }

  return pos_list;
}

}  // namespace opossum
