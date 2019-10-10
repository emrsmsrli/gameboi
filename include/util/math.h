#ifndef GAMEBOY_MATH_H
#define GAMEBOY_MATH_H

#include <cstdint>

namespace gameboy::math {
    template<typename T>
    bool bit_test(const T n, const uint32_t bit)
    {
        return ((n >> bit) & 0x1u) == 0x1u;
    }

    template<typename T>
    void bit_set(T& n, const uint32_t bit)
    {
        n |= 0x1u << bit;
    }

    template<typename T>
    void bit_reset(T& n, const uint32_t bit)
    {
        n &= ~(0x1u << bit);
    }

    // template<uint8_t HighBitCount = 8u, uint8_t LowBitCount = 8u>
    // inline uint16_t make_word(uint8_t high, uint8_t low) {
    //     return (high << LowBitCount) | (low & (1u));
    // }
}

#endif //GAMEBOY_MATH_H
