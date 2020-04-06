#ifndef GAMEBOY_ENVELOPE_H
#define GAMEBOY_ENVELOPE_H

#include "gameboy/cpu/register8.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

/**
 * Bit 7-4 - Initial Volume of envelope (0-0Fh) (0=No Sound)
 * Bit 3   - Envelope Direction (0=Decrease, 1=Increase)
 * Bit 2-0 - Number of envelope sweep (n: 0-7)
 * (If zero, stop envelope operation.)
 *
 * Length of 1 step = n*(1/64) seconds
 */
struct envelope {
    enum class direction_type {
        decrease, increase
    };

    register8 reg;

    /**
     * (If zero, stop envelope operation.)
     * @return The number of envelope sweeps.
     */
    [[nodiscard]] uint8_t sweep_count() const noexcept { return (reg.value() & 0x07u); }

    /** @return Envelope direction */
    [[nodiscard]] direction_type direction() const noexcept { return static_cast<direction_type>(bit::extract(reg, 3u)); }

    /** @return Initial volume between [0-0Fh] */
    [[nodiscard]] uint8_t initial_volume() const noexcept { return ((reg.value() >> 4u) & 0xFu); }
};

} // namespace gameboy

#endif //GAMEBOY_ENVELOPE_H
