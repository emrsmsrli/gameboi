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

    [[nodiscard]] stat_mode get_mode() const noexcept { return static_cast<stat_mode>(reg.value() & 0x03u); }
    void set_mode(const stat_mode mode) noexcept { reg = (reg & 0xFCu) | static_cast<uint8_t>(mode); }

    void set_coincidence_flag() noexcept { reg = bit::set(reg, 2u); }
    void reset_coincidence_flag() noexcept { reg = bit::reset(reg, 2u); }

    [[nodiscard]] bool coincidence_interrupt_enabled() const noexcept { return bit::test(reg, 6u); }
    [[nodiscard]] bool mode_interrupt_enabled(const stat_mode mode) const noexcept
    {
        return bit::test(reg, static_cast<uint8_t>(mode) + 3u);
    }

    [[nodiscard]] bool mode_interrupt_enabled() const noexcept { return mode_interrupt_enabled(get_mode()); }
};

} // namespace gameboy

#endif //GAMEBOY_REGISTER_STAT_H
