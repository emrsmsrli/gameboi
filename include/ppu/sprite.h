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

    [[nodiscard]] bool prioritized() const noexcept { return !math::bit_test(attributes, 7u); }
    [[nodiscard]] bool v_flipped() const noexcept { return math::bit_test(attributes, 6u); }
    [[nodiscard]] bool h_flipped() const noexcept { return math::bit_test(attributes, 5u); }

    /**
     * Bit 4
     * Palette number
     * Sprite colors are taken from OBJ1PAL if
     * this bit is set to 1 and from OBJ0PAL
     * otherwise.
     */
    [[nodiscard]] uint8_t palette_number() const noexcept { return 1; }

    [[nodiscard]] bool operator<(const sprite& other) const noexcept
    {
        return x < other.x;
    }
};

}

#endif //GAMEBOY_SPRITE_H
