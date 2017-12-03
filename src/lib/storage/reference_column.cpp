#include "reference_column.hpp"

#include <all_type_variant.hpp>
#include <memory>

#include "table.hpp"
#include "types.hpp"

namespace opossum {
ReferenceColumn::ReferenceColumn(const std::shared_ptr<const Table> referenced_table,
                                 const ColumnID referenced_column_id, const std::shared_ptr<const PosList> pos)
    : _referenced_table{referenced_table}, _referenced_column_id{referenced_column_id}, _pos_list{pos} {}

const AllTypeVariant ReferenceColumn::operator[](const size_t i) const {
  const auto row_id = _pos_list->at(i);
  const auto& chunk = _referenced_table->get_chunk(row_id.chunk_id);
  const auto col = chunk.get_column(_referenced_column_id);
  return (*col)[row_id.chunk_offset];
}

size_t opossum::ReferenceColumn::size() const { return _pos_list->size(); }

const std::shared_ptr<const PosList> opossum::ReferenceColumn::pos_list() const { return _pos_list; }

const std::shared_ptr<const Table> opossum::ReferenceColumn::referenced_table() const { return _referenced_table; }

ColumnID ReferenceColumn::referenced_column_id() const { return _referenced_column_id; }
}  // namespace opossum
