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
    register8& operator=(uint8_t val) noexcept;
    register8& operator=(const address8& val) noexcept;

    /* math ops */
    register8& operator+=(uint8_t val) noexcept;
    register8& operator-=(uint8_t val) noexcept;

    [[nodiscard]] register8 operator+(const register8& val) const noexcept;
    [[nodiscard]] uint16_t operator+(uint16_t val) const noexcept;
    [[nodiscard]] uint16_t operator-(uint16_t val) const noexcept;

    /* logical */
    register8& operator&=(uint8_t val) noexcept;
    register8& operator|=(uint8_t val) noexcept;
    register8& operator^=(uint8_t val) noexcept;

    register8 operator&(uint8_t val) const noexcept;
    register8 operator|(uint8_t val) const noexcept;
    register8 operator^(uint8_t val) const noexcept;
    register8 operator~() const noexcept;

    /* comparison */
    bool operator==(uint8_t val) const noexcept;
    bool operator==(const register8& other) const noexcept;
    bool operator>(uint8_t val) const noexcept;
    bool operator<(uint8_t val) const noexcept;
    bool operator>=(uint8_t val) const noexcept;
    bool operator>=(const register8& other) const noexcept;
    bool operator<=(uint8_t val) const noexcept;
    bool operator<=(const register8& other) const noexcept;

private:
    uint8_t bits_ = 0x00u;
};

} // namespace gameboy

#endif //GAMEBOY_REGISTER8_H
