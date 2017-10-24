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
  DebugAssert(!has_definition(name), "column definition already exists");
  m_column_names.push_back(name);
  m_column_types.push_back(type);
  m_is_instantiated.push_back(false);
}

void Table::add_column(const std::string& name, const std::string& type) {
  if (!has_definition(name)) {
    add_column_definition(name, type);
  } else {
    DebugAssert(is_new_column_valid(name, type), "column already exists");
  }
  for (Chunk& chunk : m_chunks) {
    chunk.add_column(make_shared_by_column_type<BaseColumn, ValueColumn>(type));
  }
  m_is_instantiated.at(column_id_by_name(name)) = true;
}

void Table::append(std::vector<AllTypeVariant> values) {
  if (is_last_chunk_full()) {
    create_new_chunk();
  }
  m_chunks.back().append(values);
}

void Table::create_new_chunk() {
  m_chunks.push_back(Chunk());
  Chunk& last_chunk = m_chunks.back();
  for (auto& column_type : m_column_types) {
    last_chunk.add_column(make_shared_by_column_type<BaseColumn, ValueColumn>(column_type));
  }
}

uint16_t Table::col_count() const { return m_column_names.size(); }

uint64_t Table::row_count() const { return (chunk_count() - 1) * chunk_size() + m_chunks.back().size(); }

ChunkID Table::chunk_count() const { return ChunkID{static_cast<uint32_t>(m_chunks.size())}; }

ColumnID Table::column_id_by_name(const std::string& column_name) const {
  auto count = col_count();
  for (uint16_t id = 0; id < count; id++) {
    if (m_column_names[id] == column_name) {
      return ColumnID{id};
    }
  }
  throw std::runtime_error("column does not exist");
}

uint32_t Table::chunk_size() const { return m_chunk_size; }

const std::vector<std::string>& Table::column_names() const { return m_column_names; }

const std::string& Table::column_name(ColumnID column_id) const { return m_column_names.at(column_id); }

const std::string& Table::column_type(ColumnID column_id) const { return m_column_types.at(column_id); }

Chunk& Table::get_chunk(ChunkID chunk_id) { return m_chunks.at(chunk_id); }

const Chunk& Table::get_chunk(ChunkID chunk_id) const { return m_chunks.at(chunk_id); }

bool Table::is_last_chunk_full() const { return !has_infinite_chunk_size() && m_chunks.back().size() == m_chunk_size; }

bool Table::has_infinite_chunk_size() const { return m_chunk_size == 0; }

bool Table::has_definition(const std::string& name) const {
  return std::find(m_column_names.begin(), m_column_names.end(), name) != m_column_names.end();
}

bool Table::is_new_column_valid(const std::string& name, const std::string& type) const {
  auto it = std::find(m_column_names.begin(), m_column_names.end(), name);
  auto pos = it - m_column_names.begin();
  auto existing_type = m_column_types.at(pos);
  if (existing_type != type) {
    return false;
  }
  if (m_is_instantiated.at(pos)) {
    return false;
  }
  return true;
}
}  // namespace opossum
