#include "table_scan.hpp"

#include <set>

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
}  // namespace opossum
