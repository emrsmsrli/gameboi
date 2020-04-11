#ifndef GAMEBOY_ENVELOPE_H
#define GAMEBOY_ENVELOPE_H

#include "gameboy/cpu/register8.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

struct envelope {
    enum class mode : uint8_t {
        decrease = 0, increase = 1
    };

    register8 reg;
    int32_t timer = 0u;

    [[nodiscard]] uint8_t period() const noexcept { return reg.value() & 0x07u; }
    [[nodiscard]] mode get_mode() const noexcept { return static_cast<mode>(bit::extract(reg, 3u)); }
    [[nodiscard]] uint8_t initial_volume() const noexcept { return reg.value() >> 4u; }
};

} // namespace gameboy

#endif //GAMEBOY_ENVELOPE_H
