#ifndef GAMEBOY_BG_ATTRIBUTES_H
#define GAMEBOY_BG_ATTRIBUTES_H

#include <cstdint>

#include "gameboy/util/mathutil.h"

namespace gameboy {

struct bg_attributes {
    uint8_t attributes = 0u;

    [[nodiscard]] bool prioritized() const noexcept { return !bit_test(attributes, 7u); }
    [[nodiscard]] bool v_flipped() const noexcept { return bit_test(attributes, 6u); }
    [[nodiscard]] bool h_flipped() const noexcept { return bit_test(attributes, 5u); }
    [[nodiscard]] uint8_t vram_bank() const noexcept { return extract_bit(attributes, 3u); }
    [[nodiscard]] uint8_t palette_index() const noexcept { return attributes & 0x7u; }
};

} // namespace gameboy

#endif //GAMEBOY_BG_ATTRIBUTES_H
