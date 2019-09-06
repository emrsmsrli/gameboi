#ifndef GAMEBOY_MATH_H
#define GAMEBOY_MATH_H

#include <cstdint>

namespace gameboy::math {
    template<typename T>
    inline bool bit_test(T n, uint32_t bit) {
        return ((n >> bit) & 0x1u) == 0x1u;
    }

    template<typename T>
    inline void bit_set(T& n, uint32_t bit) {
        n |= 0x1u << bit;
    }

    template<typename T>
    inline void bit_reset(T& n, uint32_t bit) {
        n &= ~(0x1u << bit);
    }
}

#endif //GAMEBOY_MATH_H
