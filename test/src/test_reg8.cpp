#include "gtest/gtest.h"
#include "gameboy/cpu/register8.h"

TEST(Register8, Assignment) {
    gameboy::register8 r(0x03);
    r = 0xDE;
    ASSERT_EQ(r.value(), 0xDE);
}

TEST(Register8, AddSub) {
    gameboy::register8 r(0x01);
    r += 0x0D;
    ASSERT_EQ(r, 0x0E);
    r -= 0x0C;
    ASSERT_EQ(r, 0x02);

    ASSERT_EQ(r, gameboy::register8(0x01) + 0x01);
    ASSERT_EQ(r, gameboy::register8(0x03) - 0x01);
}

TEST(Register8, Logical) {
    gameboy::register8 r(0x01);
    r |= 0x0D;
    ASSERT_EQ(r, 0x0D);
    r &= 0x01;
    ASSERT_EQ(r, 0x01);
    r ^= 0x01;
    ASSERT_EQ(r, 0x00);

    ASSERT_EQ(gameboy::register8(0x01) & 0x00, 0x00);
    ASSERT_EQ(gameboy::register8(0xF0) | 0x01, 0xF1);
    ASSERT_EQ(~gameboy::register8(0xF0), 0x0F);
}

TEST(Register8, Comparison) {
    gameboy::register8 r(0x01);
    ASSERT_EQ(r, 0x01);
    ASSERT_EQ(r, gameboy::register8(0x01));
    ASSERT_TRUE(r > 0x00);
    ASSERT_TRUE(r == 0x01);
    ASSERT_FALSE(r > 0x01);
}
