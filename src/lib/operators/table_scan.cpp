#include "table_scan.hpp"

#include "resolve_type.hpp"
#include "storage/table.hpp"

namespace opossum {

    TableScan::TableScan(const std::shared_ptr<const opossum::AbstractOperator> in, opossum::ColumnID column_id,
                         const opossum::ScanType scan_type, const opossum::AllTypeVariant search_value)
    : AbstractOperator(in), _column_id{column_id}, _scan_type{scan_type}, _search_value{search_value} { }

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

        const auto in_table = _input_table_left();
        const auto col_type = in_table->column_type(_column_id);

        auto _impl = make_unique_by_column_type<BaseTableScanImpl, TableScanImpl>(col_type);

        _output = _impl->on_execute(_search_value, in_table);
        return _output;
    }

    template <typename T>
    std::shared_ptr<const Table> TableScan::TableScanImpl<T>::on_execute(const AllTypeVariant & search_value, const std::shared_ptr<const Table> table) {
//        const T search_value_t = type_cast<T>(search_value);

        for (ChunkID chunk_id{0}; chunk_id < table->chunk_count(); ++chunk_id) {
//            const auto chunk = table->get_chunk(chunk_id);
//            const auto base_column = chunk.get_column(column_id);
        }

        return nullptr;
    }


} // namespace opossum
