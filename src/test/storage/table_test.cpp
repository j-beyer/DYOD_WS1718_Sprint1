﻿#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../base_test.hpp"
#include "gtest/gtest.h"

#include "../lib/resolve_type.hpp"
#include "../lib/storage/dictionary_column.hpp"
#include "../lib/storage/table.hpp"

namespace opossum {

class StorageTableTest : public BaseTest {
 protected:
  void SetUp() override {
    t.add_column("col_1", "int");
    t.add_column("col_2", "string");
  }

  Table t{2};
};

TEST_F(StorageTableTest, ChunkCount) {
  EXPECT_EQ(t.chunk_count(), 1u);
  t.append({4, "Hello,"});
  t.append({6, "world"});
  t.append({3, "!"});
  EXPECT_EQ(t.chunk_count(), 2u);
}

TEST_F(StorageTableTest, ChunkCountInfiniteSize) {
  auto t_inf = Table{};
  EXPECT_EQ(t_inf.chunk_count(), 1u);
  EXPECT_EQ(t_inf.chunk_size(), 0u);

  t_inf.add_column("col_1", "int");
  EXPECT_EQ(t_inf.col_count(), 1u);

  t_inf.append({1});
  t_inf.append({2});
  t_inf.append({3});
  EXPECT_EQ(t_inf.chunk_count(), 1u);
  EXPECT_EQ(t_inf.row_count(), 3u);
}

TEST_F(StorageTableTest, CompressChunk) {
  t.append({4, "Hello,"});
  t.append({6, "world"});
  t.compress_chunk(ChunkID{0});
  const auto& chunk_after = t.get_chunk(ChunkID{0});

  const auto column1 = chunk_after.get_column(ColumnID{0});
  const auto column2 = chunk_after.get_column(ColumnID{1});
  const auto base_column1 = std::dynamic_pointer_cast<DictionaryColumn<int>>(column1);
  const auto base_column2 = std::dynamic_pointer_cast<DictionaryColumn<std::string>>(column2);
  EXPECT_NE(base_column1, nullptr);
  EXPECT_NE(base_column2, nullptr);
}

TEST_F(StorageTableTest, CompressEmptyChunk) { EXPECT_THROW(t.compress_chunk(ChunkID{0}), std::exception); }

TEST_F(StorageTableTest, CompressNonFullChunk) {
  t.append({4, "Hello"});
  EXPECT_THROW(t.compress_chunk(ChunkID{0}), std::exception);
}

TEST_F(StorageTableTest, CompressInfiniteChunk) {
  Table t_inf{};
  t_inf.add_column("col_1", "int");
  t_inf.append({1});
  t_inf.append({2});
  EXPECT_THROW(t_inf.compress_chunk(ChunkID{0}), std::exception);
}

TEST_F(StorageTableTest, GetChunk) {
  t.get_chunk(ChunkID{0});
  // TODO(anyone): Do we want checks here?
  // EXPECT_THROW(t.get_chunk(ChunkID{q}), std::exception);
  t.append({4, "Hello,"});
  t.append({6, "world"});
  t.append({3, "!"});
  t.get_chunk(ChunkID{1});
}

TEST_F(StorageTableTest, GetChunkConst) {
  const Table ct{2};
  ct.get_chunk(ChunkID{0});
}

TEST_F(StorageTableTest, ColCount) { EXPECT_EQ(t.col_count(), 2u); }

TEST_F(StorageTableTest, RowCount) {
  EXPECT_EQ(t.row_count(), 0u);
  t.append({4, "Hello,"});
  t.append({6, "world"});
  t.append({3, "!"});
  EXPECT_EQ(t.row_count(), 3u);
}

TEST_F(StorageTableTest, GetColumnName) {
  EXPECT_EQ(t.column_name(ColumnID{0}), "col_1");
  EXPECT_EQ(t.column_name(ColumnID{1}), "col_2");
  // TODO(anyone): Do we want checks here?
  // EXPECT_THROW(t.column_name(ColumnID{2}), std::exception);
}

TEST_F(StorageTableTest, GetColumnType) {
  EXPECT_EQ(t.column_type(ColumnID{0}), "int");
  EXPECT_EQ(t.column_type(ColumnID{1}), "string");
  // TODO(anyone): Do we want checks here?
  // EXPECT_THROW(t.column_type(ColumnID{2}), std::exception);
}

TEST_F(StorageTableTest, GetColumnIdByName) {
  EXPECT_EQ(t.column_id_by_name("col_2"), 1u);
  EXPECT_THROW(t.column_id_by_name("no_column_name"), std::exception);
}

TEST_F(StorageTableTest, GetChunkSize) { EXPECT_EQ(t.chunk_size(), 2u); }

TEST_F(StorageTableTest, ColumnNames) {
  auto col_names = t.column_names();
  auto find_col_1 = std::find(col_names.cbegin(), col_names.cend(), "col_1");
  auto find_col_2 = std::find(col_names.cbegin(), col_names.cend(), "col_2");
  EXPECT_NE(find_col_1, col_names.end());
  EXPECT_NE(find_col_2, col_names.end());
  EXPECT_EQ(col_names.size(), 2u);
}

TEST_F(StorageTableTest, AddExistingColumnDefinition) {
  EXPECT_THROW(t.add_column_definition("col_1", "int"), std::exception);
}

TEST_F(StorageTableTest, AddColumnOfWrongType) {
  t.add_column_definition("col_3", "int");
  EXPECT_THROW(t.add_column("col_3", "string"), std::exception);
}

TEST_F(StorageTableTest, AddColumnTwice) {
  t.add_column("col_3", "int");
  EXPECT_THROW(t.add_column("col_3", "int"), std::exception);
}
}  // namespace opossum
