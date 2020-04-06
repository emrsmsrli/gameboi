#ifndef GAMEBOY_POLYNOMIAL_COUNTER_H
#define GAMEBOY_POLYNOMIAL_COUNTER_H

#include "gameboy/cpu/register8.h"

namespace gameboy {

/*
 * The amplitude is randomly switched between high and low at the given frequency.
 * A higher frequency will make the noise to appear 'softer'.
 * When Bit 3 is set, the output will become more regular,
 * and some frequencies will sound more like Tone than Noise.
*/
struct polynomial_counter {
    register8 reg;

    [[nodiscard]] uint8_t shift_clock_frequency() const noexcept { return (reg.value() >> 4u) & 0xFu; }
    [[nodiscard]] uint8_t counter_width() const noexcept { return (0x10u >> (bit::extract(reg, 3u))) - 1u; }
    [[nodiscard]] uint8_t dividing_ratio() const noexcept { return reg.value() & 0x7u; }
};

} // namespace gameboy

#endif //GAMEBOY_POLYNOMIAL_COUNTER_H
