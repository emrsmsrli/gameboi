#ifndef GAMEBOY_MATHUTIL_H
#define GAMEBOY_MATHUTIL_H

#include <cstdint>

namespace gameboy {

template<typename T>
bool bit_test(const T n, const uint32_t bit) noexcept
{
    return ((n >> bit) & 0x1u) == 0x1u;
}

template<typename T>
T bit_set(const T n, const uint32_t bit) noexcept
{
    return n | (0x1u << bit);
}

template<typename T>
T bit_reset(const T n, const uint32_t bit) noexcept
{
    return n & ~(0x1u << bit);
}

template<typename T>
T mask(const T n, const uint32_t m) noexcept
{
    return n & m;
}

template<typename T>
bool mask_test(const T n, const uint32_t m) noexcept
{
    return (n & m) == m;
}

template<typename T>
T mask_reset(const T n, const uint32_t m) noexcept
{
    return n & ~m;
}

template<typename T>
T mask_set(const T n, const uint32_t m) noexcept
{
    return n | m;
}

inline bool half_carry(const uint8_t x, const uint8_t y) noexcept
{
    return (((x & 0x0Fu) + (y & 0x0Fu)) & 0x10u) != 0;
}

inline bool half_carry(const uint16_t x, const uint16_t y) noexcept
{
    return (((x & 0x0FFFu) + (y & 0x0FFFu)) & 0x1000u) != 0;
}

inline bool full_carry(const uint8_t x, const uint8_t y) noexcept
{
    return (((x & 0xFFu) + (y & 0xFFu)) & 0x100u) != 0;
}

inline bool full_carry(const uint16_t x, const uint16_t y) noexcept
{
    return (((x & 0xFFFFu) + ((y & 0xFFFFu))) & 0x10000u) != 0;
}

inline bool half_borrow(const uint8_t x, const uint8_t y) noexcept
{
    return (x & 0x0Fu) < (y & 0x0Fu);
}

inline bool full_borrow(const uint8_t x, const uint8_t y) noexcept
{
    return (x & 0xFFu) < (y & 0xFFu);
}

inline uint16_t word(const uint8_t high, const uint8_t low) noexcept
{
    return static_cast<uint16_t>(high) << 8u | low;
}

} // namespace gameboy

#endif //GAMEBOY_MATHUTIL_H
