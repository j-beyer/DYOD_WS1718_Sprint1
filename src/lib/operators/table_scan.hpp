#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "abstract_operator.hpp"
#include "all_type_variant.hpp"
#include "types.hpp"
#include "utils/assert.hpp"



namespace opossum {

class BaseTableScanImpl;
class Table;

class TableScan : public AbstractOperator {
 public:
  TableScan(const std::shared_ptr<const AbstractOperator> in, ColumnID column_id, const ScanType scan_type,
            const AllTypeVariant search_value);

  ~TableScan();

  ColumnID column_id() const;
  ScanType scan_type() const;
  const AllTypeVariant& search_value() const;

 protected:
  std::shared_ptr<const Table> _on_execute() override;

  ScanType _scan_type;
  ColumnID _column_id;
  AllTypeVariant _search_value;

  struct TableScanImpl{
      virtual std::shared_ptr<opossum::Table> _on_execute(std::shared_ptr<const AbstractOperator> _in, ScanType scan_type, ColumnID column_id, AllTypeVariant search_value) = 0;
  };

  template<T>
  struct TableScanTypeImpl : TableScanImpl{
      virtual std::shared_ptr<opossum::Table> _on_execute(std::shared_ptr<const AbstractOperator> _in, ScanType scan_type, ColumnID column_id, AllTypeVariant search_value) override;

      std::shared_ptr<const AbstractOperator> in;
      ColumnID _column_id;
      ScanType _scan_type;
      T _search_value;
  };

  std::unique_ptr<TableScanImpl> _impl;
};

}  // namespace opossum
