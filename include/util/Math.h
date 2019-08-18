#ifndef GAMEBOY_MATH_H
#define GAMEBOY_MATH_H

#include <cstdint>

namespace gameboy::math {
    inline bool bit_test(uint8_t n, uint8_t bit) {
        return ((n >> bit) & 0x1) == 0x1;
    }

    inline void bit_set(uint8_t& n, uint8_t bit) {
        n |= 0x1 << bit;
    }

    inline void bit_reset(uint8_t& n, uint8_t bit) {
        n &= ~(0x1 << bit);
    }
}

#endif //GAMEBOY_MATH_H
