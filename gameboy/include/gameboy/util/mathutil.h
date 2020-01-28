#ifndef GAMEBOY_MATHUTIL_H
#define GAMEBOY_MATHUTIL_H

#include <cstdint>

namespace gameboy {

template<typename T>
constexpr bool mask_test(const T n, const uint32_t m) noexcept
{
    return (n & m) == m;
}

template<typename T>
constexpr bool bit_test(const T n, const uint32_t bit) noexcept
{
    const auto mask = 0x1u << bit;
    return (n & mask) == mask;
}

template<typename T>
constexpr T bit_set(const T n, const uint32_t bit) noexcept
{
    return n | (0x1u << bit);
}

template<typename T>
constexpr T bit_reset(const T n, const uint32_t bit) noexcept
{
    return n & ~(0x1u << bit);
}

template<typename T>
constexpr T bit_flip(const T n, const uint32_t bit) noexcept
{
    return n ^ (0x1u << bit);
}

constexpr bool half_carry(const uint8_t x, const uint8_t y) noexcept
{
    return mask_test((x & 0x0Fu) + (y & 0x0Fu), 0x10u);
}

constexpr bool half_carry(const uint16_t x, const uint16_t y) noexcept
{
    return mask_test((x & 0x0FFFu) + (y & 0x0FFFu), 0x1000u);
}

constexpr bool full_carry(const uint8_t x, const uint8_t y) noexcept
{
    return mask_test(static_cast<uint16_t>(x) + static_cast<uint16_t>(y), 0x100u);
}

constexpr bool full_carry(const uint16_t x, const uint16_t y) noexcept
{
    return mask_test(x + y, 0x10000u);
}

constexpr bool half_borrow(const uint8_t x, const uint8_t y) noexcept
{
    return (x & 0x0Fu) < (y & 0x0Fu);
}

constexpr bool full_borrow(const uint8_t x, const uint8_t y) noexcept
{
    return x < y;
}

inline uint16_t word(const uint8_t high, const uint8_t low) noexcept
{
    return static_cast<uint16_t>(high) << 8u | low;
}

template<typename T>
uint8_t extract_bit(const T value, const uint32_t bit)
{
    return static_cast<uint8_t>(bit_test(value, bit));
}

} // namespace gameboy

#endif //GAMEBOY_MATHUTIL_H
