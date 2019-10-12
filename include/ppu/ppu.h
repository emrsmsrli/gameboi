#ifndef GAMEBOY_PPU_H
#define GAMEBOY_PPU_H

#include <memory>

#include <memory/mmu.h>

namespace gameboy {

class ppu {
public:
    explicit ppu(std::shared_ptr<mmu> memory_management_unit);

    void tick(uint8_t cycles);

private:
    enum mode : uint8_t {
        /**
         * CPU can access both the display RAM (8000h-9FFFh)
         * and OAM (FE00h-FE9Fh)
         */
            h_blank = 0u,

        /**
         * The LCD contoller is in the V-Blank period (or the
         * display is disabled) and the CPU can access both the
         * display RAM (8000h-9FFFh) and OAM (FE00h-FE9Fh)
         */
            v_blank = 1u,

        /**
         * The LCD controller is reading from OAM memory.
         * The CPU <cannot> access OAM memory (FE00h-FE9Fh)
         * during this period.
         */
            reading_oam = 2u,

        /**
         * The LCD controller is reading from both OAM and VRAM,
         * The CPU <cannot> access OAM and VRAM during this period.
         * CGB Mode: Cannot access Palette Data (FF69,FF6B) either.
         */
            reading_oam_vram = 3u
    };

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

    enum status_flag : uint8_t {
        coincidence_interrupt = 6u,
        reading_oam_interrupt = 5u,
        vblank_interrupt = 4u,
        hblank_interrupt = 3u,
        coincidence_flag = 2u,  // (0:LYC<>LY, 1:LYC=LY)
    };

    std::shared_ptr<mmu> mmu_;

    uint16_t cycle_count_;
    mode mode_{mode::h_blank};

    [[nodiscard]] bool is_control_flag_set(control_flag flag) const;
};

}

#endif //GAMEBOY_PPU_H
