#include <gtest/gtest.h>

#include "gameboy/cpu/register16.h"

TEST(Register16, HighLow) {
    gameboy::register16 r(0xF003);
    ASSERT_EQ(r.high().value(), 0xF0);
    ASSERT_EQ(r.low().value(), 0x03);
}

TEST(Register16, Assignment) {
    gameboy::register16 r(0x0003);
    r = 0xDE00;
    ASSERT_EQ(r.value(), 0xDE00);
}

TEST(Register16, IncDec) {
    gameboy::register16 r(0x0000);
    ++r;
    ++r;
    ASSERT_EQ(r, 0x0002);
    --r;
    ASSERT_EQ(r, 0x0001);

    gameboy::register16 r2(0x0000);
    ASSERT_EQ(r2++, 0x0000);
    ASSERT_EQ(r2, 0x0001);

    ASSERT_EQ(r2--, 0x0001);
    ASSERT_EQ(r2, 0x0000);
}

TEST(Register16, AddSub) {
    gameboy::register16 r(0x00FF);
    r += 0x0001;
    ASSERT_EQ(r, 0x0100);
    r -= 0x000C;
    ASSERT_EQ(r, 0x00F4);

    ASSERT_EQ(r, gameboy::register16(0x00F0) + 0x0004);
    ASSERT_EQ(r, gameboy::register16(0x00F5) - 0x0001);
}

TEST(Register16, Logical) {
    gameboy::register16 r(0x0001);
    r = r | 0x000D;
    ASSERT_EQ(r, 0x000D);
    r = r & 0x0001;
    ASSERT_EQ(r, 0x0001);
    r = r ^ 0x0001;
    ASSERT_EQ(r, 0x0000);

    ASSERT_EQ(gameboy::register16(0x0001) & 0x0000, 0x0000);
    ASSERT_EQ(gameboy::register16(0x00F0) | 0x0001, 0x00F1);
    ASSERT_EQ(~gameboy::register16(0xFFDE), 0x0021);
}

TEST(Register16, Comparison) {
    gameboy::register16 r(0x0001);
    ASSERT_EQ(r, 0x0001);
    ASSERT_EQ(r, gameboy::register16(0x0001));
    ASSERT_TRUE(r.value() > 0x00);
    ASSERT_TRUE(r.value() >= 0x0000);
    ASSERT_TRUE(r.value() == 0x0001);
    ASSERT_FALSE(r.value() > 0x0001);
}