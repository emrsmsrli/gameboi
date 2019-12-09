#ifndef GAMEBOY_REGISTER_STAT_H
#define GAMEBOY_REGISTER_STAT_H

#include "gameboy/cpu/register8.h"

namespace gameboy {

enum class stat_mode : uint8_t {
    h_blank = 0u,
    v_blank = 1u,
    reading_oam = 2u,
    reading_oam_vram = 3u
};

struct register_stat {
    register8 reg;

    explicit register_stat(const uint8_t val)
        : reg{val} {}

    [[nodiscard]] stat_mode get_mode() const noexcept
    {
        return static_cast<stat_mode>(reg.value() & 0x03u);
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
