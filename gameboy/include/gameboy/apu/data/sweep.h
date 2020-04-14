//
// Created by Emre on 06/04/2020.
//

#ifndef GAMEBOY_SWEEP_H
#define GAMEBOY_SWEEP_H

#include "gameboy/cpu/register8.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

struct sweep {
    enum class mode : uint8_t {
        increase = 0, decrease = 1
    };

    register8 reg;

    bool enabled = false;

    int16_t timer = 0;
    uint16_t shadow = 0u;

    [[nodiscard]] uint8_t period() const noexcept { return (reg.value() >> 4u) & 0x7u; }
    [[nodiscard]] mode get_mode() const noexcept { return static_cast<mode>(bit::extract(reg, 3u)); }
    [[nodiscard]] uint8_t shift_count() const noexcept { return reg.value() & 0x7u; }
};

} // namespace gameboy

#endif //GAMEBOY_SWEEP_H
