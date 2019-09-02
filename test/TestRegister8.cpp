#include <cpu/Register8.h>
#include <gtest/gtest.h>

TEST(Register8, Assignment) {
    gameboy::cpu::Register8 r(0x03);
    r = 0xDE;
    ASSERT_EQ(r.get_value(), 0xDE);
}

TEST(Register8, IncDec) {
    gameboy::cpu::Register8 r(0x00);
    ++r;
    ++r;
    ASSERT_EQ(r, 0x02);
    --r;
    ASSERT_EQ(r, 0x01);
}

TEST(Register8, AddSub) {
    gameboy::cpu::Register8 r(0x01);
    r += 0x0D;
    ASSERT_EQ(r, 0x0E);
    r -= 0x0C;
    ASSERT_EQ(r, 0x02);

    ASSERT_EQ(r, gameboy::cpu::Register8(0x01) + gameboy::cpu::Register8(0x01));
    ASSERT_EQ(r, gameboy::cpu::Register8(0x03) - gameboy::cpu::Register8(0x01));
}

TEST(Register8, Logical) {
    gameboy::cpu::Register8 r(0x01);
    r |= 0x0D;
    ASSERT_EQ(r, 0x0D);
    r &= 0x01;
    ASSERT_EQ(r, 0x01);
    r ^= 0x01;
    ASSERT_EQ(r, 0xFD);

    ASSERT_EQ(gameboy::cpu::Register8(0x01) & gameboy::cpu::Register8(0x00), 0x00);
    ASSERT_EQ(gameboy::cpu::Register8(0xF0) | gameboy::cpu::Register8(0x01), 0xF1);
    ASSERT_EQ(~gameboy::cpu::Register8(0xF0), 0x0F);
}

TEST(Register8, Comparison) {
    gameboy::cpu::Register8 r(0x01);
    ASSERT_EQ(r, 0x01);
    ASSERT_EQ(r, gameboy::cpu::Register8(0x01));
    ASSERT_TRUE(r > 0x00);
    ASSERT_TRUE(r >= 0x00);
    ASSERT_TRUE(r == 0x01);
    ASSERT_FALSE(r > 0x01);
}
