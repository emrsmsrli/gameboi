
#include <Register.h>
#include "gtest/gtest.h"

TEST(Register, HighLowEqual) {
    gameboy::Register r;
    r = 0x0F0F;
    ASSERT_EQ(r.get_high(), 0x0F);
    ASSERT_EQ(r.get_high(), r.get_low());
}