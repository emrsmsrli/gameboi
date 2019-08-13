
#include <Register.h>
#include "gtest/gtest.h"

TEST(HalfRegister, Test) {
    gameboy::Register r(0x0300);
    ASSERT_TRUE(r.get_high().test(0));
    ASSERT_TRUE(r.get_high().test(1));
    r = 0x0100;
    ASSERT_TRUE(r.get_high().test(0));
    ASSERT_FALSE(r.get_high().test(1));
}

TEST(HalfRegister, Set) {
    gameboy::Register r(0x0000);
    r.get_high().set(4);
    ASSERT_EQ(r.get_high(), 0x10);
    r.get_low().set(0);
    ASSERT_EQ(r.get_low(), 0x01);
}

TEST(HalfRegister, Reset) {
    gameboy::Register r(0x0300);
    r.get_high().reset(0);
    ASSERT_EQ(r.get_high(), 0x02);
}

TEST(HalfRegister, Flip) {
    gameboy::Register r(0x0000);
    r.get_high().flip(0);
    ASSERT_EQ(r.get_high(), 0x01);
    r.get_high().flip(0);
    ASSERT_EQ(r.get_high(), 0x00);
}

TEST(HalfRegister, Assignment) {
    gameboy::Register r(0xEECB);
    r.get_low() = 0xDE;
    ASSERT_EQ(r, 0xEEDE);
}

TEST(HalfRegister, IncDec) {
    gameboy::Register r(0x0000);
    ++r.get_low();
    ++r.get_low();
    ASSERT_EQ(r, 0x0002);
    --r.get_low();
    ASSERT_EQ(r, 0x0001);
}

TEST(HalfRegister, AddSub) {
    gameboy::Register r(0x0001);
    r.get_low() += 0x0D;
    ASSERT_EQ(r, 0x000E);
    r.get_low() -= 0x0C;
    ASSERT_EQ(r, 0x0002);
}
