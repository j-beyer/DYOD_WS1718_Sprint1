#include "table.hpp"

#include <algorithm>
#include <iomanip>
#include <limits>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "value_column.hpp"

#include "resolve_type.hpp"
#include "types.hpp"
#include "utils/assert.hpp"

namespace opossum {

Table::Table(const uint32_t chunk_size) : m_chunk_size{chunk_size} { create_new_chunk(); }

void Table::add_column_definition(const std::string& name, const std::string& type) {
  m_column_names.push_back(name);
  m_column_types.push_back(type);
}

void Table::add_column(const std::string& name, const std::string& type) {
  add_column_definition(name, type);
  for (Chunk& chunk : m_chunks) {
    chunk.add_column(make_shared_by_column_type<BaseColumn, ValueColumn>(type));
  }
}

void Table::append(std::vector<AllTypeVariant> values) {
  if (m_chunks.size() == 0 || m_chunks.back().size() >= chunk_size()) {
    create_new_chunk();
  }
  m_chunks.back().append(values);
}

void Table::create_new_chunk() {
  m_chunks.push_back(Chunk());
  // TODO(florian): add the needed columns to the chunk
}

uint16_t Table::col_count() const { return m_column_names.size(); }

uint64_t Table::row_count() const {
  if (m_chunks.size() == 0) {
    return 0;
  }
  return (chunk_count() - 1) * chunk_size() + m_chunks.back().size();
}

ChunkID Table::chunk_count() const {
  ChunkID ret = static_cast<ChunkID>(m_chunks.size());
  return ret;
}

ColumnID Table::column_id_by_name(const std::string& column_name) const {
  for (size_t id = 0; id < m_column_names.size(); id++) {
    if (m_column_names[id] == column_name) {
      return static_cast<ColumnID>(id);
    }
  }
  // TODO(florian): find out what to do when the column does not exist (do ids start at 0?)
  return ColumnID{0};
}

uint32_t Table::chunk_size() const { return m_chunk_size; }

const std::vector<std::string>& Table::column_names() const {
  throw std::runtime_error("Implement Table::column_names()");
}

const std::string& Table::column_name(ColumnID column_id) const {
  throw std::runtime_error("Implement Table::column_name");
}

const std::string& Table::column_type(ColumnID column_id) const {
  throw std::runtime_error("Implement Table::column_type");
}

Chunk& Table::get_chunk(ChunkID chunk_id) { throw std::runtime_error("Implement Table::get_chunk"); }

const Chunk& Table::get_chunk(ChunkID chunk_id) const { throw std::runtime_error("Implement Table::get_chunk"); }

}  // namespace opossum
