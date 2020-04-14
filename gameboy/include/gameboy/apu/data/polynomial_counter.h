#ifndef GAMEBOY_POLYNOMIAL_COUNTER_H
#define GAMEBOY_POLYNOMIAL_COUNTER_H

#include "gameboy/cpu/register8.h"

namespace gameboy {

struct polynomial_counter {
    register8 reg;

    [[nodiscard]] uint8_t shift_clock_frequency() const noexcept { return reg.value() >> 4u; }
    [[nodiscard]] bool has_7_bit_counter_width() const noexcept { return bit::test(reg, 3u); }
    [[nodiscard]] uint8_t dividing_ratio() const noexcept { return reg.value() & 0x7u; }
};

} // namespace gameboy

#endif //GAMEBOY_POLYNOMIAL_COUNTER_H
