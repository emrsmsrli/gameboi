#ifndef GAMEBOY_FREQUENCY_H
#define GAMEBOY_FREQUENCY_H

#include "gameboy/cpu/register8.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

// todo maybe reg16 is better suited?
struct frequency {
    /** frequency lo (8 bits) */
    register8 low;

    /**
     * Bit 7   - Initial (1=Restart Sound)     (Write Only)
     * Bit 6   - Counter/consecutive selection (Read/Write)
     *           (1=Stop output when length in NR11 expires)
     * Bit 2-0 - Frequency's higher 3 bits (x) (Write Only)
     *
     * Frequency = 131072/(2048-x) Hz
     */
    register8 high;

    [[nodiscard]] uint16_t value() const noexcept { return word(low.value(), high.value()); }
};

} // namespace gameboy

#endif //GAMEBOY_FREQUENCY_H
