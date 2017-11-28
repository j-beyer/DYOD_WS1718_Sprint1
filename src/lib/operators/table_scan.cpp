
#include "resolve_type.hpp"
#include "table_scan.hpp"

namespace opossum {

    TableScan::TableScan(const std::shared_ptr<const opossum::AbstractOperator> in, opossum::ColumnID column_id,
                         const opossum::ScanType scan_type, const opossum::AllTypeVariant search_value)
    : _in{in}, _column_id{column_id}, _scan_type{scan_type}, _search_value{search_value} { }

    ColumnID TableScan::column_id() const {
        return _column_id;
    }

    ScanType TableScan::scan_type() const {
        return _scan_type;
    }

    const AllTypeVariant& TableScan::search_value() const {
        return _search_value;
    }

    std::shared_ptr<const Table> TableScan::_on_execute() {

        const auto & in_table = _in->get_output();
        const auto & col_type = in_table.column_type(_column_id);

        _impl = make_unique_by_column_type<BaseTableScanImpl, TableScanImpl>(col_type);

        return _impl->_on_execute();
    }


} // namespace opossum
