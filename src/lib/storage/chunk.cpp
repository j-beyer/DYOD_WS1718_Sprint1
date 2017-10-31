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
  DebugAssert(column->size() == this->size() || m_columns.size() == 0, "column size does not match chunk size");
  m_columns.push_back(column);
}

void Chunk::append(const std::vector<AllTypeVariant>& values) {
  DebugAssert(values.size() == m_columns.size(), "number of given values does not match number of columns");
  for (size_t i = 0; i < values.size(); i++) {
    m_columns[i]->append(values[i]);
  }
}

std::shared_ptr<BaseColumn> Chunk::get_column(ColumnID column_id) const { return m_columns.at(column_id); }

uint16_t Chunk::col_count() const { return m_columns.size(); }

uint32_t Chunk::size() const {
  if (col_count() == 0) {
    return 0;
  }
  // all columns have the same size (as per add_column() and append() implementation)
  // so we can just return the size of the first column
  return m_columns[0]->size();
}

}  // namespace opossum
