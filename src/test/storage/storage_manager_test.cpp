#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "../base_test.hpp"
#include "gtest/gtest.h"

#include "../lib/storage/storage_manager.hpp"
#include "../lib/storage/table.hpp"

namespace opossum {

class StorageStorageManagerTest : public BaseTest {
 protected:
  void SetUp() override {
    auto& sm = StorageManager::get();
    auto t1 = std::make_shared<Table>();
    auto t2 = std::make_shared<Table>(4);

    sm.add_table("first_table", t1);
    sm.add_table("second_table", t2);
  }
};

TEST_F(StorageStorageManagerTest, GetTable) {
  auto& sm = StorageManager::get();
  auto t3 = sm.get_table("first_table");
  auto t4 = sm.get_table("second_table");
  EXPECT_THROW(sm.get_table("third_table"), std::exception);
}

TEST_F(StorageStorageManagerTest, DropTable) {
  auto& sm = StorageManager::get();
  sm.drop_table("first_table");
  EXPECT_THROW(sm.get_table("first_table"), std::exception);
  EXPECT_THROW(sm.drop_table("first_table"), std::exception);
}

TEST_F(StorageStorageManagerTest, ResetTable) {
  StorageManager::reset();
  auto& sm = StorageManager::get();
  EXPECT_THROW(sm.get_table("first_table"), std::exception);
}

TEST_F(StorageStorageManagerTest, DoesNotHaveTable) {
  auto& sm = StorageManager::get();
  EXPECT_EQ(sm.has_table("third_table"), false);
}

TEST_F(StorageStorageManagerTest, HasTable) {
  auto& sm = StorageManager::get();
  EXPECT_EQ(sm.has_table("first_table"), true);
}

TEST_F(StorageStorageManagerTest, TableNames) {
  auto& sm = StorageManager::get();
  auto names = sm.table_names();
  EXPECT_EQ(names.size(), 2u);
  EXPECT_EQ(names.front(), "first_table");
  EXPECT_EQ(names.back(), "second_table");
}

TEST_F(StorageStorageManagerTest, Print) {
  auto& sm = StorageManager::get();
  sm.drop_table("second_table");
  auto stream = std::ostringstream{};
  sm.print(stream);
  auto str = stream.str();
  EXPECT_EQ(str, "name: first_table\t#columns: 0\t#rows: 0\t#chunks: 1\tchunk size: 0\n\n");
}

}  // namespace opossum
