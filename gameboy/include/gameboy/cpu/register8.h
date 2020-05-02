#ifndef GAMEBOY_REGISTER8_H
#define GAMEBOY_REGISTER8_H

#include <cstdint>

#include "gameboy/memory/addressfwd.h"

namespace gameboy {

/**
 * Represents an 8-bit register in the CPU
 */
class register8 {
public:
    register8() = default;

    explicit register8(const uint8_t default_value) noexcept
        : bits_{default_value} {}

    [[nodiscard]] uint8_t value() const noexcept { return bits_; }

    /* assignment */
    register8& operator=(const uint8_t val) noexcept { bits_ = val; return *this; }
    register8& operator=(const address8& val) noexcept;

    /* math ops */
    register8& operator+=(const uint8_t val) noexcept
    {
        bits_ += val;
        return *this;
    }

    register8& operator-=(const uint8_t val) noexcept
    {
        bits_ -= val;
        return *this;
    }

    [[nodiscard]] register8 operator+(const register8& other) const noexcept { return register8(bits_ + other.value()); }
    [[nodiscard]] uint16_t operator+(const uint16_t val) const noexcept { return bits_ + val; }
    [[nodiscard]] uint16_t operator-(const uint16_t val) const noexcept { return bits_ - val; }

    /* logical */
    register8& operator&=(const uint8_t val) noexcept
    {
        bits_ &= val;
        return *this;
    }

    register8& operator|=(const uint8_t val) noexcept
    {
        bits_ |= val;
        return *this;
    }

    register8& operator^=(const uint8_t val) noexcept
    {
        bits_ ^= val;
        return *this;
    }

    [[nodiscard]] register8 operator&(const uint8_t val) const noexcept { return register8(bits_ & val); }
    [[nodiscard]] register8 operator|(const uint8_t val) const noexcept { return register8(bits_ | val); }
    [[nodiscard]] register8 operator^(const uint8_t val) const noexcept { return register8(bits_ ^ val); }
    [[nodiscard]] register8 operator~() const noexcept { return register8(~bits_); }

    /* comparison */
    bool operator==(const uint8_t val) const noexcept { return bits_ == val; }
    bool operator==(const register8& other) const noexcept { return bits_ == other.bits_; }
    bool operator!=(const uint8_t val) const noexcept { return bits_ != val; }
    bool operator!=(const register8& other) const noexcept { return bits_ != other.value(); }
    bool operator>(const uint8_t val) const noexcept { return bits_ > val; }
    bool operator>(const register8& other) const noexcept { return bits_ > other.value(); }
    bool operator<(const uint8_t val) const noexcept { return bits_ < val; }
    bool operator<(const register8& other) const noexcept { return bits_ < other.value(); }
    bool operator>=(const uint8_t val) const noexcept { return bits_ >= val; }
    bool operator>=(const register8& other) const noexcept { return bits_ >= other.value(); }
    bool operator<=(const uint8_t val) const noexcept { return bits_ <= val; }
    bool operator<=(const register8& other) const noexcept { return bits_ <= other.value(); }

private:
    uint8_t bits_ = 0x00u;
};

} // namespace gameboy

#endif //GAMEBOY_REGISTER8_H
