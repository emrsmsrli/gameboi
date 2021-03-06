#ifndef GAMEBOY_ATTRIBUTES_H
#define GAMEBOY_ATTRIBUTES_H

#include <cstdint>

#include "gameboy/util/mathutil.h"

namespace gameboy::attributes {

struct uninitialized {};

struct bg {
    uint8_t attributes = 0u;

    [[nodiscard]] bool prioritized() const noexcept { return bit::test(attributes, 7u); }
    [[nodiscard]] bool v_flipped() const noexcept { return bit::test(attributes, 6u); }
    [[nodiscard]] bool h_flipped() const noexcept { return bit::test(attributes, 5u); }
    [[nodiscard]] uint8_t vram_bank() const noexcept { return bit::extract(attributes, 3u); }
    [[nodiscard]] uint8_t palette_index() const noexcept { return attributes & 0x7u; }
};

struct obj {
    uint8_t y = 0u;
    uint8_t x = 0u;
    uint8_t tile_number = 0u;
    uint8_t attributes = 0u;

    [[nodiscard]] bool prioritized() const noexcept { return !bit::test(attributes, 7u); }
    [[nodiscard]] bool v_flipped() const noexcept { return bit::test(attributes, 6u); }
    [[nodiscard]] bool h_flipped() const noexcept { return bit::test(attributes, 5u); }
    [[nodiscard]] uint8_t gb_palette_index() const noexcept { return bit::extract(attributes, 4u); }
    [[nodiscard]] uint8_t vram_bank() const noexcept { return bit::extract(attributes, 3u); }
    [[nodiscard]] uint8_t cgb_palette_index() const noexcept { return attributes & 0x7u; }
};

} // namespace gameboy::attributes

#endif //GAMEBOY_ATTRIBUTES_H
