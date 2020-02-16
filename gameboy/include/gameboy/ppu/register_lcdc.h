#ifndef GAMEBOY_REGISTER_LCDC_H
#define GAMEBOY_REGISTER_LCDC_H

#include "gameboy/cpu/register8.h"

namespace gameboy {

struct register_lcdc {
    register8 reg;

    explicit register_lcdc(const uint8_t val)
        : reg{val} {}

    [[nodiscard]] bool lcd_enabled() const noexcept { return bit_test(reg, 7u); }
    [[nodiscard]] bool window_enabled() const noexcept { return bit_test(reg, 5u); }
    [[nodiscard]] bool large_obj() const noexcept { return bit_test(reg, 2u); }
    [[nodiscard]] bool obj_enabled() const noexcept { return bit_test(reg, 1u); }

    /**
     * LCDC.0 - 2) CGB in CGB Mode: BG and Window Master Priority
     * When Bit 0 is cleared, the background and window lose their priority - the sprites will be always
     * displayed on top of background and window, independently of the priority flags in OAM and BG Map attributes.
     *
     * LCDC.0 - 3) CGB in Non CGB Mode: BG and Window Display
     * When Bit 0 is cleared, both background and window become blank (white),
     * ie. the Window Display Bit (Bit 5) is ignored in that case.
     * Only Sprites may still be displayed (if enabled in Bit 1).
     * This is a possible compatibility problem - any monochrome games (if any) that disable the background,
     * but still want to display the window wouldn't work properly on CGBs.
     */
    [[nodiscard]] bool bg_enabled() const noexcept { return bit_test(reg, 0u); }

    /**
     * primary   : address16(0x9800u)
     * secondary : address16(0x9C00u)
     */
    [[nodiscard]] bool window_map_secondary() const noexcept { return bit_test(reg, 6u); }
    [[nodiscard]] bool unsigned_mode() const noexcept { return bit_test(reg, 4u); }
    [[nodiscard]] bool bg_map_secondary() const noexcept { return bit_test(reg, 3u); }
};

} // namespace gameboy

#endif //GAMEBOY_REGISTER_LCDC_H
