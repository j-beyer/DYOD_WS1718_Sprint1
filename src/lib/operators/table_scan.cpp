#include "table_scan.hpp"

#include "resolve_type.hpp"
#include "storage/dictionary_column.hpp"
#include "storage/reference_column.hpp"
#include "storage/table.hpp"
#include "storage/value_column.hpp"

namespace opossum {

TableScan::TableScan(const std::shared_ptr<const opossum::AbstractOperator> in, opossum::ColumnID column_id,
                     const opossum::ScanType scan_type, const opossum::AllTypeVariant search_value)
    : AbstractOperator(in), _column_id{column_id}, _scan_type{scan_type}, _search_value{search_value} {}

ColumnID TableScan::column_id() const { return _column_id; }

ScanType TableScan::scan_type() const { return _scan_type; }

const AllTypeVariant& TableScan::search_value() const { return _search_value; }

std::shared_ptr<const Table> TableScan::_on_execute() {
  const auto in_table = _input_table_left();
  const auto column_type = in_table->column_type(_column_id);

  auto _impl = make_unique_by_column_type<BaseTableScanImpl, TableScanImpl>(column_type);

  _output = _impl->on_execute(_search_value, in_table, _column_id);
  return _output;
}

template <typename T>
std::shared_ptr<const Table> TableScan::TableScanImpl<T>::on_execute(const AllTypeVariant& search_value_variant,
                                                                     const std::shared_ptr<const Table> table,
                                                                     const ColumnID column_id) {
  //        const T search_value = type_cast<T>(search_value_variant);

  for (ChunkID chunk_id{0}; chunk_id < table->chunk_count(); ++chunk_id) {
    const auto& chunk = table->get_chunk(chunk_id);
    const auto base_column = chunk.get_column(column_id);

    auto value_column = std::dynamic_pointer_cast<ValueColumn<T>>(base_column);
    if (value_column != nullptr) {
      return nullptr;
    }

    auto dictionary_column = std::dynamic_pointer_cast<DictionaryColumn<T>>(base_column);
    if (dictionary_column != nullptr) {
      return nullptr;
    }

    auto reference_column = std::dynamic_pointer_cast<ReferenceColumn>(base_column);
    if (reference_column != nullptr) {
      return nullptr;
    }

    throw std::runtime_error("Invalid column subclass!");
  }

  return nullptr;
}

}  // namespace opossum
