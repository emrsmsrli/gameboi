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

    [[nodiscard]] uint8_t shift_clock_frequency() const noexcept { return reg.value() >> 4u; }
    [[nodiscard]] bool has_7_bit_counter_width() const noexcept { return bit::test(reg, 3u); }
    [[nodiscard]] uint8_t dividing_ratio() const noexcept { return reg.value() & 0x7u; }
};

} // namespace gameboy

#endif //GAMEBOY_POLYNOMIAL_COUNTER_H
