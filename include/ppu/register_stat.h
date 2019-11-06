#ifndef GAMEBOY_REGISTER_STAT_H
#define GAMEBOY_REGISTER_STAT_H

#include <cpu/register8.h>

namespace gameboy {

enum class stat_mode : uint8_t {
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

struct register_stat {
    register8 reg;

    [[nodiscard]] stat_mode get_mode() const noexcept
    {
        return static_cast<stat_mode>(mask(reg.value(), 0x03u));
    }

    void set_mode(stat_mode mode) noexcept
    {
        reg = mask_set(reg, static_cast<uint8_t>(mode));
    }

    [[nodiscard]] bool mode_interrupt_set() const noexcept
    {
        return bit_test(reg, 1u << static_cast<uint8_t>(get_mode()));
    }

    [[nodiscard]] bool coincidence_flag_set() const noexcept { return bit_test(reg, 2u); }
    void set_coincidence_flag() const noexcept { bit_set(reg, 2u); }
    void reset_coincidence_flag() const noexcept { bit_reset(reg, 2u); }
};

} // namespace gameboy

#endif //GAMEBOY_REGISTER_STAT_H