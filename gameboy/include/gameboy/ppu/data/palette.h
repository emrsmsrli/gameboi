#ifndef GAMEBOY_PALETTE_H
#define GAMEBOY_PALETTE_H

#include <array>

#include "gameboy/ppu/color.h"

namespace gameboy {

struct palette {
    std::array<color, 4> colors;

    constexpr palette() = default;
    constexpr explicit palette(const color& color)
	    : palette{color, color, color, color} {}
    constexpr palette(const color& c_1, const color& c_2, const color& c_3, const color& c_4)
	    : colors{c_1, c_2, c_3, c_4} {}

    [[nodiscard]] constexpr static palette from(const palette& reference, const uint8_t data) noexcept
    {
        palette p{};

        for(uint8_t i = 0; i < 4; ++i) {
            const auto index = (data >> (i * 2u)) & 0x3u;
            p.colors[i] = reference.colors[index];
        }

        return p;
    }
};

} // namespace gameboy

#endif //GAMEBOY_PALETTE_H
