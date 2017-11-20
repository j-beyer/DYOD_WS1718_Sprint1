#include <iomanip>
#include <iterator>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "base_column.hpp"
#include "chunk.hpp"

#include "utils/assert.hpp"

namespace opossum {

void Chunk::add_column(std::shared_ptr<BaseColumn> column) {
  // if the chunk is empty, always allow adding a new column
  // otherwise, only allow if column size matches
  Assert(column->size() == this->size() || _columns.size() == 0, "Column size does not match chunk size!");
  _columns.push_back(column);
}

void Chunk::append(const std::vector<AllTypeVariant>& values) {
  Assert(values.size() == _columns.size(), "Number of given values does not match number of columns!");
  for (size_t i = 0; i < values.size(); i++) {
    _columns[i]->append(values[i]);
  }
}

std::shared_ptr<BaseColumn> Chunk::get_column(ColumnID column_id) const { return _columns.at(column_id); }

uint16_t Chunk::col_count() const { return _columns.size(); }

uint32_t Chunk::size() const {
  if (col_count() == 0) {
    return 0;
  }
  // all columns have the same size (as per add_column() and append() implementation)
  // so we can just return the size of the first column
  return _columns[0]->size();
}

}  // namespace opossum
