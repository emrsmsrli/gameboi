#ifndef GAMEBOY_CONTROL_REGISTER_H
#define GAMEBOY_CONTROL_REGISTER_H

#include <cpu/register8.h>

namespace gameboy {

enum control_flag : uint8_t {
    lcd_enable = 7,
    window_tile_map_display_select = 6u, // todo (0=9800-9BFF, 1=9C00-9FFF)
    window_display_enable = 5u,
    bg_window_tile_data_select = 4u,     // todo (0=8800-97FF, 1=8000-8FFF)
    bg_tile_map_display_select = 3u,     // todo (0=9800-9BFF, 1=9C00-9FFF)
    sprite_size = 2u,
    sprite_display_enable = 1u,
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
    bg_display_enable = 0u
};

struct register_lcdc {
    register8 reg;
};

} // namespace gameboy

#endif //GAMEBOY_CONTROL_REGISTER_H
