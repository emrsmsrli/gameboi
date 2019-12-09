#ifndef GAMEBOY_REGISTER_LCDC_H
#define GAMEBOY_REGISTER_LCDC_H

#include "gameboy/cpu/register8.h"
#include "gameboy/memory/address.h"

namespace gameboy {

struct register_lcdc {
    register8 reg;

    explicit register_lcdc(const uint8_t val)
        : reg{val} {}

    [[nodiscard]] bool lcd_enabled() const noexcept
    {
        return bit_test(reg, 7u);
    }

    [[nodiscard]] bool window_enabled() const noexcept
    {
        return bit_test(reg, 5u);
    }

    [[nodiscard]] bool large_obj() const noexcept
    {
        return bit_test(reg, 2u);
    }

    [[nodiscard]] bool obj_enabled() const noexcept
    {
        return bit_test(reg, 1u);
    }

    /**
     * LCDC.0 - 1) Monochrome Gameboy and SGB: BG Display
     * When Bit 0 is cleared, the background becomes blank (white).
     * Window and Sprites may still be displayed (if enabled in Bit 1 and/or Bit 5).
     *
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
    [[nodiscard]] bool bg_enabled() const noexcept
    {
        return bit_test(reg, 0u);
    }

    [[nodiscard]] address16 window_map_address() const noexcept
    {
        return bit_test(reg, 6u)
               ? address16(0x9C00u)
               : address16(0x9800u);
    }

    [[nodiscard]] address16 tile_base_address() const noexcept
    {
        return bit_test(reg, 4u)
               ? address16(0x8000u)
               : address16(0x8800u);
    }

    [[nodiscard]] address16 bg_map_address() const noexcept
    {
        return bit_test(reg, 3u)
               ? address16(0x9C00u)
               : address16(0x9800u);
    }
};

} // namespace gameboy

#endif //GAMEBOY_REGISTER_LCDC_H
