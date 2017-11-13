
#include <memory>

#include "../base_test.hpp"
#include "gtest/gtest.h"

#include "../lib/storage/fitted_attribute_vector.hpp"

namespace opossum {

class StorageFittedAttributeVectorTest : public BaseTest {
 protected:
  void SetUp() override {
    fav = std::make_shared<FittedAttributeVector<uint8_t>>(3);
    fav->set(0, ValueID{0});
    fav->set(1, ValueID{1});
    fav->set(2, ValueID{1});
  }

  std::shared_ptr<FittedAttributeVector<uint8_t>> fav;
};

TEST_F(StorageFittedAttributeVectorTest, SetAndGet) {
  EXPECT_EQ(fav->get(0), ValueID{0});
  EXPECT_EQ(fav->get(1), ValueID{1});
  EXPECT_EQ(fav->get(2), ValueID{1});
}

TEST_F(StorageFittedAttributeVectorTest, SetOutOfBounds) { EXPECT_THROW(fav->set(4, ValueID{4}), std::exception); }

TEST_F(StorageFittedAttributeVectorTest, SizeTooLargeForWidth) {
  EXPECT_THROW(std::make_shared<FittedAttributeVector<uint8_t>>(300), std::exception);
}

TEST_F(StorageFittedAttributeVectorTest, Size) { EXPECT_EQ(fav->size(), 3u); }

TEST_F(StorageFittedAttributeVectorTest, Width) {
  EXPECT_EQ(fav->width(), 1u);
  auto fav16 = std::make_shared<FittedAttributeVector<uint16_t>>(1);
  EXPECT_EQ(fav16->width(), 2u);
  auto fav32 = std::make_shared<FittedAttributeVector<uint32_t>>(1);
  EXPECT_EQ(fav32->width(), 4u);
}
}  // namespace opossum
