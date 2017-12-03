#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "abstract_operator.hpp"
#include "all_type_variant.hpp"
#include "storage/dictionary_column.hpp"
#include "types.hpp"
#include "utils/assert.hpp"

namespace opossum {

class BaseTableScanImpl {
 public:
  virtual std::shared_ptr<const Table> on_execute() = 0;
};
class Table;

class TableScan : public AbstractOperator {
 public:
  TableScan(const std::shared_ptr<const AbstractOperator> in, ColumnID column_id, const ScanType scan_type,
            const AllTypeVariant search_value);

  ~TableScan() = default;

  ColumnID column_id() const;
  ScanType scan_type() const;
  const AllTypeVariant& search_value() const;

 protected:
  std::shared_ptr<const Table> _on_execute() override;

  template <typename T>
  class TableScanImpl : public BaseTableScanImpl {
   public:
    TableScanImpl<T>(const std::shared_ptr<const Table> in_table, ColumnID column_id, const ScanType scan_type,
                     const AllTypeVariant& search_value)
        : _in_table{in_table}, _column_id{column_id}, _scan_type{scan_type}, _search_value{search_value} {
      _pos_list = std::make_shared<PosList>();
    }

    std::shared_ptr<const Table> on_execute() override;

   protected:
    void _create_pos_list(bool is_reference);
    template <typename C>
    Comparator<C> _get_comparator() const;
    bool _should_prune_chunk(const T& search_value, const std::shared_ptr<DictionaryColumn<T>> dictionary_column);
    std::vector<ChunkOffset> _eval_operator(const T& search_value, const std::vector<T>& values,
                                            std::function<bool(const T&, const T&)> compare_function) const;
    std::vector<ChunkOffset> _eval_operator(const T& search_value,
                                            const std::shared_ptr<DictionaryColumn<T>> dictionary_column) const;

    const std::shared_ptr<const Table> _in_table;
    ColumnID _column_id;
    const ScanType _scan_type;
    const AllTypeVariant _search_value;

    std::shared_ptr<PosList> _pos_list;
  };

  const std::shared_ptr<const AbstractOperator> _in;
  ColumnID _column_id;
  const ScanType _scan_type;
  const AllTypeVariant _search_value;
};

}  // namespace opossum
