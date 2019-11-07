#ifndef GAMEBOY_PALETTE_H
#define GAMEBOY_PALETTE_H

#include <array>

#include <ppu/color.h>
#include <util/mathutil.h>

namespace gameboy {

struct palette {
    std::array<color, 4> colors = {
        color{255, 255, 255},
        color{192, 192, 192},
        color{96, 96, 96},
        color{0, 0, 0},
    };

    [[nodiscard]] constexpr palette from(uint8_t data) const noexcept
    {
        palette p{};

        for(uint8_t i = 0; i < 4; ++i) {
            const auto index = mask(data, 0x3u << i) >> i;
            p.colors[i] = colors[index];
        }

        return p;
    }
};

} // namespace gameboy

#endif //GAMEBOY_PALETTE_H
