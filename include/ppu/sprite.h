#ifndef GAMEBOY_SPRITE_H
#define GAMEBOY_SPRITE_H

#include <cstdint>

#include <util/mathutil.h>

namespace gameboy {

struct sprite {
    uint8_t y;
    uint8_t x;
    uint8_t character_code;
    uint8_t attributes;

    [[nodiscard]] bool prioritized() const noexcept { return !bit_test(attributes, 7u); } // todo?
    [[nodiscard]] bool v_flipped() const noexcept { return bit_test(attributes, 6u); }
    [[nodiscard]] bool h_flipped() const noexcept { return bit_test(attributes, 5u); }
    [[nodiscard]] uint8_t vram_bank() const noexcept { return bit_test(attributes, 3u) ? 0x1u : 0x0u; } // todo cgb only
    [[nodiscard]] uint8_t palette_number() const noexcept { return mask(attributes, 0x7u); } // todo cgb only

    [[nodiscard]] bool operator<(const sprite& other) const noexcept
    {
        return x < other.x;
    }
};

} // namespace gameboy

#endif //GAMEBOY_SPRITE_H
