
#include "table_scan.hpp"
#include "type_cast.hpp"
#include "storage/table.hpp"
#include "storage/value_column.hpp"
#include "storage/dictionary_column.hpp"
#include "storage/storage_manager.hpp"

namespace opossum {

TableScan::TableScan(const std::shared_ptr<const AbstractOperator> in, ColumnID column_id, const ScanType scan_type, const AllTypeVariant search_value)
: _in{in},
_column_id{column_id},
_scan_type{scan_type},
_search_value{search_value}
{
    // Get the T
    // Return the implementation of T
     _search_value.


}

TableScan::~TableScan()
{

}

ColumnID TableScan::column_id() const
{
    return _column_id;
}

ScanType TableScan::scan_type() const
{
    return _scan_type;
}

const AllTypeVariant &TableScan::search_value() const
{
    return _search_value;
}

std::shared_ptr<const Table> TableScan::_on_execute()
{
    _impl._on_execute();
}

template<T>
std::shared_ptr<Table> TableScan::TableScanTypeImpl::_on_execute(std::shared_ptr<const AbstractOperator> _in, ScanType scan_type, ColumnID column_id, AllTypeVariant search_value)
{
    const std::shared_ptr<Table> in_table = _in->get_output();

    std::shared_ptr<Table> out_table = Table();

    const auto& col_type = in_table->column_type(column_id);

    // TODO: think about how we get from T to string
    out_table->add_column("results", col_type);

    out_table->get_chunk(ChunkID{0}).get_column(ColumnID{0})->

    const auto& _search_value = type_cast<T>(search_value);



    const auto& chunk_count = in_table->chunk_count();

    for(ChunkID chunk_ID{0}; chunk_ID < chunk_count; ++chunk_ID){
        // get the chunk
        const auto& chunk = in_table->get_chunk(chunkd_ID);

        // get the column
        const std::shared_pointer<BaseColumn> base_column = chunk.get_column(column_id);

        // TODO(Florian): think about how we differentiate between problems with column type and with T mismatch
        const auto vc = std::dynamic_pointer_cast<ValueColumn<T>>(base_column);

        if(vc != nullptr){
            // Congratulations, it's a value column
            const auto& values = vc->values();

            for(const auto& val : values){
                switch(scan_type){
                    ScanType::OpEquals:
                        if(val == search_value)
                        break;
                    ScanType::OpGreaterThan:
                        break;
                    ScanType::OpGreaterThanEquals:
                        break;
                    ScanType::OpLessThan:
                        break;
                    ScanType::OpLessThanEquals:
                        break;
                    ScanType::OpNotEquals:
                        break;
                }
            }

//            switch(scan_type){
//                ScanType::OpEquals:
//                    break;
//                ScanType::OpGreaterThan:
//                    break;
//                ScanType::OpGreaterThanEquals:
//                    break;
//                ScanType::OpLessThan:
//                    break;
//                ScanType::OpLessThanEquals:
//                    break;
//                ScanType::OpNotEquals:
//                    break;
//            }


        }


        // switch on chunk type

    }

}


} // namespace opossum
