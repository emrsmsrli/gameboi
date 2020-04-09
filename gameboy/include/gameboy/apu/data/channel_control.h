#ifndef GAMEBOY_CHANNEL_CONTROL_H
#define GAMEBOY_CHANNEL_CONTROL_H

#include "gameboy/cpu/register8.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

struct channel_control {
    enum class terminal {
        right, left
    };

    register8 nr_50;
    register8 nr_51;

    [[nodiscard]] bool vin_routed_to_terminal(const terminal t) const noexcept
    {
        return bit::test(nr_50, terminal_bit_offset(t) + 3u);
    }

    template<typename T>
    [[nodiscard]] T terminal_volume(const terminal t) const noexcept
    {
        return static_cast<T>((nr_50.value() >> terminal_bit_offset(t)) & 0x7u);
    }

    [[nodiscard]] bool channel_enabled_on_terminal(const uint8_t channel_no, const terminal t) const noexcept
    {
        return bit::test(nr_51, terminal_bit_offset(t) + channel_no);
    }

    [[nodiscard]] static uint32_t terminal_bit_offset(const terminal t) noexcept { return static_cast<uint32_t>(t) * 4u; }
};

} // namespace gameboy

#endif //GAMEBOY_CHANNEL_CONTROL_H
