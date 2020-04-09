#ifndef GAMEBOY_REGISTER_LCDC_H
#define GAMEBOY_REGISTER_LCDC_H

#include "gameboy/cpu/register8.h"

namespace gameboy {

struct register_lcdc {
    register8 reg;

    explicit register_lcdc(const uint8_t val)
        : reg{val} {}

    [[nodiscard]] bool lcd_enabled() const noexcept { return bit::test(reg, 7u); }
    [[nodiscard]] bool window_enabled() const noexcept { return bit::test(reg, 5u); }
    [[nodiscard]] bool large_obj() const noexcept { return bit::test(reg, 2u); }
    [[nodiscard]] bool obj_enabled() const noexcept { return bit::test(reg, 1u); }
    [[nodiscard]] bool bg_enabled() const noexcept { return bit::test(reg, 0u); }
    [[nodiscard]] bool window_map_secondary() const noexcept { return bit::test(reg, 6u); }
    [[nodiscard]] bool unsigned_mode() const noexcept { return bit::test(reg, 4u); }
    [[nodiscard]] bool bg_map_secondary() const noexcept { return bit::test(reg, 3u); }
};

} // namespace gameboy

#endif //GAMEBOY_REGISTER_LCDC_H
