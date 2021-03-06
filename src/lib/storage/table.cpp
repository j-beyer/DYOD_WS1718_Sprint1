﻿#include "table.hpp"

#include <algorithm>
#include <iomanip>
#include <limits>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "dictionary_column.hpp"
#include "value_column.hpp"

#include "resolve_type.hpp"
#include "types.hpp"
#include "utils/assert.hpp"

namespace opossum {

Table::Table(const uint32_t chunk_size) : _chunk_size{chunk_size} { create_new_chunk(); }

void Table::add_column_definition(const std::string& name, const std::string& type) {
  Assert(!_has_definition(name), "Column definition already exists!");
  _column_names.push_back(name);
  _column_types.push_back(type);
  _is_instantiated.push_back(false);
}

void Table::add_column(const std::string& name, const std::string& type) {
  if (!_has_definition(name)) {
    add_column_definition(name, type);
  } else {
    _validate_existing_definition(name, type);
  }
  for (Chunk& chunk : _chunks) {
    chunk.add_column(make_shared_by_column_type<BaseColumn, ValueColumn>(type));
  }
  _is_instantiated.at(column_id_by_name(name)) = true;
}

void Table::append(std::vector<AllTypeVariant> values) {
  if (_is_last_chunk_full()) {
    create_new_chunk();
  }
  _chunks.back().append(values);
}

void Table::create_new_chunk() {
  auto new_chunk = Chunk();

  for (const auto& column_type : _column_types) {
    new_chunk.add_column(make_shared_by_column_type<BaseColumn, ValueColumn>(column_type));
  }

  _chunks.push_back(std::move(new_chunk));
}

void Table::compress_chunk(ChunkID chunk_id) {
  auto new_chunk = Chunk{};

  auto& old_chunk = get_chunk(chunk_id);

  // These assertions are based on Slide 11 of Week 3: "Dictionary encoding is applied to full chunk"
  Assert(!_has_infinite_chunk_size(), "Cannot compress chunk of unlimited size!");
  Assert(old_chunk.size() == chunk_size(), "Non-full chunk cannot be compressed!");

  for (ColumnID id{0}; id < old_chunk.col_count(); ++id) {
    auto cur_column = old_chunk.get_column(id);

    auto new_column = make_shared_by_column_type<BaseColumn, DictionaryColumn>(column_type(id), cur_column);
    new_chunk.add_column(new_column);
  }

  std::swap(old_chunk, new_chunk);
}

uint16_t Table::col_count() const { return _column_names.size(); }

uint64_t Table::row_count() const {
  uint64_t result = 0;
  for (const auto& chunk : _chunks) {
    result += chunk.size();
  }
  return result;
}

ChunkID Table::chunk_count() const { return ChunkID{static_cast<uint32_t>(_chunks.size())}; }

ColumnID Table::column_id_by_name(const std::string& column_name) const {
  auto count = col_count();
  for (ColumnID id{0}; id < count; ++id) {
    if (_column_names[id] == column_name) {
      return id;
    }
  }
  throw std::runtime_error("Column does not exist!");
}

uint32_t Table::chunk_size() const { return _chunk_size; }

const std::vector<std::string>& Table::column_names() const { return _column_names; }

const std::string& Table::column_name(ColumnID column_id) const { return _column_names.at(column_id); }

const std::string& Table::column_type(ColumnID column_id) const { return _column_types.at(column_id); }

Chunk& Table::get_chunk(ChunkID chunk_id) { return _chunks.at(chunk_id); }

const Chunk& Table::get_chunk(ChunkID chunk_id) const { return _chunks.at(chunk_id); }

bool Table::_is_last_chunk_full() const { return !_has_infinite_chunk_size() && _chunks.back().size() == _chunk_size; }

bool Table::_has_infinite_chunk_size() const { return _chunk_size == 0; }

bool Table::_has_definition(const std::string& name) const {
  return std::find(_column_names.cbegin(), _column_names.cend(), name) != _column_names.end();
}

void Table::_validate_existing_definition(const std::string& name, const std::string& type) const {
  auto it = std::find(_column_names.cbegin(), _column_names.cend(), name);
  auto pos = it - _column_names.cbegin();

  auto existing_type = _column_types.at(pos);
  Assert(existing_type == type, "A column definition with the same name but a different type was already added!");
  Assert(!_is_instantiated.at(pos), "A column with the given name was already added!");
}

void emplace_chunk(Chunk chunk) {
  // Implementation goes here
}

}  // namespace opossum
