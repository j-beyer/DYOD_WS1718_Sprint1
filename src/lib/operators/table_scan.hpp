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

class BaseTableScanImpl {
public:
    virtual std::shared_ptr<const Table> on_execute(const AllTypeVariant & search_value, const std::shared_ptr<const Table> table, const ColumnID column_id) = 0;
};
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


    template <typename T>
  class TableScanImpl : public BaseTableScanImpl {
  public:
      std::shared_ptr<const Table> on_execute(const AllTypeVariant & search_value_variant, const std::shared_ptr<const Table> table, const ColumnID column_id) override;
  };

    ColumnID _column_id;
    ScanType _scan_type;
    AllTypeVariant _search_value;
};

}  // namespace opossum
