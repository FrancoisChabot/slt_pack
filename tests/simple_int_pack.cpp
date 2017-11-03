#include "slt\rect_pack.h"
#include "gtest/gtest.h"

TEST(slt_rect_pack, basic_api) {
  slt::RectPacker2D<int> packer({ 12,12 });

  auto result = packer.pack({ 2, 2 });
  EXPECT_EQ(0, result.x);
  EXPECT_EQ(0, result.y);
  EXPECT_EQ(2, result.width);
  EXPECT_EQ(2, result.height);
}

TEST(slt_rect_pack, snug_fit) {
  slt::RectPacker2D<int> packer({ 12,12 });

  auto result = packer.pack({ 12, 12 });
  EXPECT_EQ(0, result.x);
  EXPECT_EQ(0, result.y);
  EXPECT_EQ(12, result.width);
  EXPECT_EQ(12, result.height);
}

TEST(slt_rect_pack, trivial_failure) {
  slt::RectPacker2D<int> packer({ 12,12 });

  auto result = packer.pack({ 13, 13 });
  EXPECT_EQ(0, result.x);
  EXPECT_EQ(0, result.y);
  EXPECT_EQ(0, result.width);
  EXPECT_EQ(0, result.height);
}

TEST(slt_rect_pack, full_fill) {
  slt::RectPacker2D<int> packer({ 5, 5 });

  bool matrix[5][5] = { false };
  for (int i = 0; i < 5 * 5; ++i) {
    auto result = packer.pack({ 1, 1 });
    EXPECT_FALSE(matrix[result.x][result.y]);
    matrix[result.x][result.y] = true;
    EXPECT_EQ(1, result.width);
    EXPECT_EQ(1, result.height);
  }
  //we should be full now
  auto result = packer.pack({ 1, 1 });
  EXPECT_EQ(0, result.width);
  EXPECT_EQ(0, result.height);
}