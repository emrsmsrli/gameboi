
#include <Register.h>
#include "gtest/gtest.h"

TEST(Register, Test) {
    gameboy::Register r;
    ASSERT_FALSE(r.test(0));
    r.set(0);
    ASSERT_TRUE(r.test(0));
}

TEST(Register, Set) {
    gameboy::Register r;
    r.set(0);
    ASSERT_EQ(r, 0x0001);
}

TEST(Register, Reset) {
    gameboy::Register r(0x0001);
    r.reset(0);
    ASSERT_EQ(r, 0x0000);
}

TEST(Register, Flip) {
    gameboy::Register r(0x0001);
    r.flip(0);
    ASSERT_EQ(r, 0x0000);
    r.flip(0);
    ASSERT_EQ(r, 0x0001);
}

TEST(Register, All) {
    gameboy::Register r(0x0000);
    ASSERT_FALSE(r.all());
    r = 0xFFFF;
    ASSERT_TRUE(r.all());
}

TEST(Register, None) {
    gameboy::Register r(0x0000);
    ASSERT_TRUE(r.none());
    r = 0xFFFF;
    ASSERT_FALSE(r.none());
}

TEST(Register, Any) {
    gameboy::Register r(0x0D00);
    ASSERT_TRUE(r.any());
    r = 0x0000;
    ASSERT_FALSE(r.any());
}

TEST(Register, IncDec) {
    gameboy::Register r(0x0000);
    ++r;
    ASSERT_EQ(r, 0x0001);
    --r;
    ASSERT_EQ(r, 0x0000);
}

TEST(Register, AddSub) {
    gameboy::Register r(0x0000);
    r += 0xF;
    ASSERT_EQ(r, 0x000F);
    r += 0x1;
    ASSERT_EQ(r, 0x0010);
    r -= 0x10;
    ASSERT_EQ(r, 0x0000);
}

TEST(Register, HighLowEqual) {
    gameboy::Register r(0x0F0F);
    ASSERT_EQ(r.get_high(), 0x0F);
    ASSERT_EQ(r.get_high(), r.get_low());
}

TEST(Register, ModifyHighLowModifiesFull) {
    gameboy::Register r(0xFFFF);
    r.get_high().reset(7);
    ASSERT_EQ(r, 0x7FFF);

    r = 0x0;
    r.get_low().set(1);
    ASSERT_EQ(r, 0x0002);
}