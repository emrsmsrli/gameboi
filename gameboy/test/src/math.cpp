#include "gtest/gtest.h"
#include "gameboy/util/mathutil.h"

TEST(math, mask_test) {
    ASSERT_TRUE(gameboy::mask_test(0x100, 0x100));
    ASSERT_FALSE(gameboy::mask_test(0x100, 0x01));
    ASSERT_TRUE(gameboy::mask_test(0xFC, 0x0C));
    ASSERT_TRUE(gameboy::mask_test(0x111, 0x110));
}

TEST(math, mask_reset) {
    ASSERT_EQ(gameboy::mask_reset(0x100, 0x100), 0x0);
    ASSERT_EQ(gameboy::mask_reset(0x100, 0x01), 0x100);
    ASSERT_EQ(gameboy::mask_reset(0xFC, 0x0C), 0xF0);
    ASSERT_EQ(gameboy::mask_reset(0x111, 0x110), 0x1);
}

TEST(math, bit_test) {
    ASSERT_FALSE(gameboy::bit_test(0x0, 7));
    ASSERT_TRUE(gameboy::bit_test(0xFC, 15));
    ASSERT_TRUE(gameboy::bit_test(0xFC, 14));
    ASSERT_FALSE(gameboy::bit_test(0xFC, 0));
    ASSERT_FALSE(gameboy::bit_test(0xFC, 1));
}

TEST(math, bit_reset) {
    ASSERT_EQ(gameboy::bit_reset(0x01, 0), 0x0);
    ASSERT_EQ(gameboy::bit_reset(0xFC, 15), 0x7C);
    ASSERT_EQ(gameboy::bit_reset(0xFC, 14), 0xBC);
    ASSERT_EQ(gameboy::bit_reset(0xFC, 0), 0xFC);
    ASSERT_EQ(gameboy::bit_reset(0xFC, 1), 0xFC);
}

TEST(math, extract_bit) {
    ASSERT_EQ(gameboy::extract_bit(0x0F, 0), 1);
    ASSERT_EQ(gameboy::extract_bit(0x0F, 1), 1);
    ASSERT_EQ(gameboy::extract_bit(0x0F, 2), 1);
    ASSERT_EQ(gameboy::extract_bit(0x0F, 3), 1);
    ASSERT_EQ(gameboy::extract_bit(0x0F, 4), 0);
    ASSERT_EQ(gameboy::extract_bit(0x0F, 5), 0);
    ASSERT_EQ(gameboy::extract_bit(0x0F, 6), 0);
    ASSERT_EQ(gameboy::extract_bit(0x0F, 7), 0);
}
