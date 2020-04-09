#include "gtest/gtest.h"
#include "gameboy/util/mathutil.h"

TEST(math, mask_test) {
    ASSERT_TRUE(gameboy::mask::test(0x100, 0x100));
    ASSERT_FALSE(gameboy::mask::test(0x100, 0x01));
    ASSERT_TRUE(gameboy::mask::test(0xFC, 0x0C));
    ASSERT_TRUE(gameboy::mask::test(0x111, 0x110));
}

TEST(math, bit_test) {
    ASSERT_FALSE(gameboy::bit::test(0x0, 7));
    ASSERT_TRUE(gameboy::bit::test(0xFC, 7));
    ASSERT_TRUE(gameboy::bit::test(0xFC, 6));
    ASSERT_FALSE(gameboy::bit::test(0xFC, 0));
    ASSERT_FALSE(gameboy::bit::test(0xFC, 1));
}

TEST(math, bit_reset) {
    ASSERT_EQ(gameboy::bit::reset(0x01, 0), 0x0);
    ASSERT_EQ(gameboy::bit::reset(0xFC, 7), 0x7C);
    ASSERT_EQ(gameboy::bit::reset(0xFC, 6), 0xBC);
    ASSERT_EQ(gameboy::bit::reset(0xFC, 0), 0xFC);
    ASSERT_EQ(gameboy::bit::reset(0xFC, 1), 0xFC);
}

TEST(math, extract_bit) {
    ASSERT_EQ(gameboy::bit::extract(0x0F, 0), 1);
    ASSERT_EQ(gameboy::bit::extract(0x0F, 1), 1);
    ASSERT_EQ(gameboy::bit::extract(0x0F, 2), 1);
    ASSERT_EQ(gameboy::bit::extract(0x0F, 3), 1);
    ASSERT_EQ(gameboy::bit::extract(0x0F, 4), 0);
    ASSERT_EQ(gameboy::bit::extract(0x0F, 5), 0);
    ASSERT_EQ(gameboy::bit::extract(0x0F, 6), 0);
    ASSERT_EQ(gameboy::bit::extract(0x0F, 7), 0);
}

TEST(math, bit_flip) {
    ASSERT_EQ(gameboy::bit::flip(0b1001, 0), 0b1000);
    ASSERT_EQ(gameboy::bit::flip(0b1001, 1), 0b1011);
    ASSERT_EQ(gameboy::bit::flip(0b1001, 2), 0b1101);
    ASSERT_EQ(gameboy::bit::flip(0b1001, 3), 0b0001);
}
