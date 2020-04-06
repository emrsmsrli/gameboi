//
// Created by Emre on 06/04/2020.
//

#ifndef GAMEBOY_SWEEP_H
#define GAMEBOY_SWEEP_H

#include "gameboy/cpu/register8.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

/**
 * Bit 6-4 - Sweep Time
 * Bit 3   - Sweep Increase/Decrease
 *            0: Addition    (frequency increases)
 *            1: Subtraction (frequency decreases)
 * Bit 2-0 - Number of sweep shift (n: 0-7)
 *
 * Sweep Time:
 * 000: sweep off - no freq change
 * 001: 7.8 ms  (1/128Hz)
 * 010: 15.6 ms (2/128Hz)
 * 011: 23.4 ms (3/128Hz)
 * 100: 31.3 ms (4/128Hz)
 * 101: 39.1 ms (5/128Hz)
 * 110: 46.9 ms (6/128Hz)
 * 111: 54.7 ms (7/128Hz)
 *
 * The change of frequency (NR13,NR14) at each shift is calculated by
 * the following formula where X(0) is initial freq & X(t-1) is last freq:
 *   X(t) = X(t-1) +/- X(t-1)/2^n
 */
struct sweep {
    enum class freq_sweep_direction {
        increase,
        decrease
    };

    register8 reg;

    [[nodiscard]] uint8_t sweep_time() const noexcept { return (reg.value() >> 4u) & 0x7u; }
    [[nodiscard]] freq_sweep_direction direction() const noexcept { return static_cast<freq_sweep_direction>(bit::extract(reg, 3u)); }
    [[nodiscard]] uint8_t count() const noexcept { return reg.value() & 0x7u; }
};

} // namespace gameboy

#endif //GAMEBOY_SWEEP_H
