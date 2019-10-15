#ifndef GAMEBOY_MATHUTIL_H
#define GAMEBOY_MATHUTIL_H

#include <cstdint>

namespace gameboy::math {

template<typename T>
bool bit_test(const T n, const uint32_t bit) noexcept
{
    return ((n >> bit) & 0x1u) == 0x1u;
}

template<typename T>
void bit_set(T& n, const uint32_t bit) noexcept
{
    n |= 0x1u << bit;
}

template<typename T>
void bit_reset(T& n, const uint32_t bit) noexcept
{
    n &= ~(0x1u << bit);
}

}

#endif //GAMEBOY_MATHUTIL_H
