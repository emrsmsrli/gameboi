#ifndef GAMEBOY_BG_H
#define GAMEBOY_BG_H

#include <cstdint>

#include "gameboy/util/mathutil.h"

namespace gameboy {

struct bg {
    uint8_t tile_number;
    uint8_t attributes;

    [[nodiscard]] bool prioritized() const noexcept { return !bit_test(attributes, 7u); } // todo?
    [[nodiscard]] bool v_flipped() const noexcept { return bit_test(attributes, 6u); }
    [[nodiscard]] bool h_flipped() const noexcept { return bit_test(attributes, 5u); }
    [[nodiscard]] uint8_t vram_bank() const noexcept { return bit_test(attributes, 3u) ? 0x1u : 0x0u; } // todo cgb only
    [[nodiscard]] uint8_t palette_index() const noexcept { return attributes & 0x7u; } // todo cgb only
};

} // namespace gameboy

#endif //GAMEBOY_BG_H
