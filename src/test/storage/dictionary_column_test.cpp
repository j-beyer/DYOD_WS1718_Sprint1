#include <memory>
#include <string>

#include "gtest/gtest.h"

#include "../../lib/all_type_variant.hpp"
#include "../../lib/resolve_type.hpp"
#include "../../lib/storage/base_column.hpp"
#include "../../lib/storage/dictionary_column.hpp"
#include "../../lib/storage/fitted_attribute_vector.hpp"
#include "../../lib/storage/value_column.hpp"
#include "../base_test.hpp"

namespace opossum {

class StorageDictionaryColumnTest : public BaseTest {
 protected:
  void SetUp() {
    vc_int = std::make_shared<ValueColumn<int>>();
    vc_str = std::make_shared<ValueColumn<std::string>>();

    vc_str->append("Bill");
    vc_str->append("Steve");
    vc_str->append("Alexander");
    vc_str->append("Steve");
    vc_str->append("Hasso");
    vc_str->append("Bill");

    for (int i = 0; i <= 10; i += 2) vc_int->append(i);

    auto base_col = make_shared_by_column_type<BaseColumn, DictionaryColumn>("string", vc_str);
    dc_str = std::dynamic_pointer_cast<DictionaryColumn<std::string>>(base_col);
  }
  std::shared_ptr<ValueColumn<int>> vc_int;
  std::shared_ptr<ValueColumn<std::string>> vc_str;
  std::shared_ptr<DictionaryColumn<std::string>> dc_str;
};

TEST_F(StorageDictionaryColumnTest, CompressAlreadyCompressedColumn) {
  EXPECT_THROW((make_shared_by_column_type<BaseColumn, DictionaryColumn>("string", dc_str)), std::exception);
}

TEST_F(StorageDictionaryColumnTest, AppendToDictionaryColumn) { EXPECT_THROW(dc_str->append("Linus"), std::exception); }

TEST_F(StorageDictionaryColumnTest, Get) {
  EXPECT_EQ(dc_str->get(0), "Bill");
  EXPECT_EQ(dc_str->get(1), "Steve");
  EXPECT_EQ(dc_str->get(2), "Alexander");
  EXPECT_EQ((*dc_str)[3], AllTypeVariant{"Steve"});
  EXPECT_EQ((*dc_str)[4], AllTypeVariant{"Hasso"});
  EXPECT_EQ((*dc_str)[5], AllTypeVariant{"Bill"});
}

TEST_F(StorageDictionaryColumnTest, FittedAttributeVectorSize) {
  auto vc_8 = std::make_shared<ValueColumn<int>>();
  auto vc_16 = std::make_shared<ValueColumn<int>>();
  auto vc_32 = std::make_shared<ValueColumn<int>>();

  for (int i = 0; i <= 10; ++i) vc_8->append(i);
  for (int i = 0; i <= 300; ++i) vc_16->append(i);
  for (int i = 0; i <= 66000; ++i) vc_32->append(i);

  auto bc_8 = make_shared_by_column_type<BaseColumn, DictionaryColumn>("int", vc_8);
  auto dc_8 = std::dynamic_pointer_cast<DictionaryColumn<int>>(bc_8);
  auto bc_16 = make_shared_by_column_type<BaseColumn, DictionaryColumn>("int", vc_16);
  auto dc_16 = std::dynamic_pointer_cast<DictionaryColumn<int>>(bc_16);
  auto bc_32 = make_shared_by_column_type<BaseColumn, DictionaryColumn>("int", vc_32);
  auto dc_32 = std::dynamic_pointer_cast<DictionaryColumn<int>>(bc_32);

  auto base_attr_8 = dc_8->attribute_vector();
  auto base_attr_16 = dc_16->attribute_vector();
  auto base_attr_32 = dc_32->attribute_vector();

  auto attr_8 = std::dynamic_pointer_cast<const FittedAttributeVector<uint8_t>>(base_attr_8);
  auto attr_16 = std::dynamic_pointer_cast<const FittedAttributeVector<uint16_t>>(base_attr_16);
  auto attr_32 = std::dynamic_pointer_cast<const FittedAttributeVector<uint32_t>>(base_attr_32);

  EXPECT_NE(attr_8, nullptr);
  EXPECT_NE(attr_16, nullptr);
  EXPECT_NE(attr_32, nullptr);
}

TEST_F(StorageDictionaryColumnTest, CompressColumnString) {
  auto dict = dc_str->dictionary();
  auto base_attr = dc_str->attribute_vector();
  auto attr = std::dynamic_pointer_cast<const FittedAttributeVector<uint8_t>>(base_attr);

  // Test attribute_vector size
  EXPECT_EQ(dc_str->size(), 6u);
  EXPECT_EQ(attr->size(), 6u);

  // Test dictionary size (uniqueness)
  EXPECT_EQ(dc_str->unique_values_count(), 4u);
  EXPECT_EQ(dict->size(), 4u);

  // Test sorting
  EXPECT_EQ((*dict)[0], "Alexander");
  EXPECT_EQ((*dict)[1], "Bill");
  EXPECT_EQ((*dict)[2], "Hasso");
  EXPECT_EQ((*dict)[3], "Steve");
}

TEST_F(StorageDictionaryColumnTest, LowerUpperBound) {
  auto col = make_shared_by_column_type<BaseColumn, DictionaryColumn>("int", vc_int);
  auto dict_col = std::dynamic_pointer_cast<DictionaryColumn<int>>(col);

  EXPECT_EQ(dict_col->lower_bound(4), ValueID{2});
  EXPECT_EQ(dict_col->upper_bound(4), ValueID{3});

  EXPECT_EQ(dict_col->lower_bound(5), ValueID{3});
  EXPECT_EQ(dict_col->upper_bound(5), ValueID{3});

  EXPECT_EQ(dict_col->lower_bound(15), INVALID_VALUE_ID);
  EXPECT_EQ(dict_col->upper_bound(15), INVALID_VALUE_ID);
}

TEST_F(StorageDictionaryColumnTest, LowerUpperBoundVariants) {
  auto col = make_shared_by_column_type<BaseColumn, DictionaryColumn>("int", vc_int);
  auto dict_col = std::dynamic_pointer_cast<DictionaryColumn<int>>(col);

  EXPECT_EQ(dict_col->lower_bound(AllTypeVariant{4}), ValueID{2});
  EXPECT_EQ(dict_col->upper_bound(AllTypeVariant{4}), ValueID{3});

  EXPECT_EQ(dict_col->lower_bound(AllTypeVariant{5}), ValueID{3});
  EXPECT_EQ(dict_col->upper_bound(AllTypeVariant{5}), ValueID{3});

  EXPECT_EQ(dict_col->lower_bound(AllTypeVariant{15}), INVALID_VALUE_ID);
  EXPECT_EQ(dict_col->upper_bound(AllTypeVariant{15}), INVALID_VALUE_ID);
}

}  // namespace opossum
