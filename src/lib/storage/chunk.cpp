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
  if (column->size() != this->size() && m_columns.size() > 0) {
    throw std::runtime_error("column size does not match chunk size");
  }
  m_columns.push_back(column);
}

void Chunk::append(std::vector<AllTypeVariant> values) {
  if (values.size() != m_columns.size()) {
    throw std::runtime_error("number of given values does not match number of columns");
  }
  for (size_t i = 0; i < values.size(); i++) {
    m_columns[i]->append(values[i]);
  }
  // Implementation goes here
}

std::shared_ptr<BaseColumn> Chunk::get_column(ColumnID column_id) const {
  // Implementation goes here
  return m_columns.at(column_id);
}

uint16_t Chunk::col_count() const {
  // Implementation goes here
  return m_columns.size();
}

uint32_t Chunk::size() const {
  // Implementation goes here
  if (col_count() == 0) {
    return 0;
  }
  return m_columns.at(0)->size();
}

}  // namespace opossum
