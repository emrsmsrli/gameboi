#ifndef GAMEBOY_REGISTER8_H
#define GAMEBOY_REGISTER8_H

#include <cstdint>

#include <memory/addressfwd.h>

namespace gameboy {

/**
 * Represents an 8-bit register in the CPU
 */
class register8 {
public:
    register8() = default;

    explicit register8(const uint8_t default_value)
            :bits(default_value) { }

    [[nodiscard]] uint8_t get_value() const { return bits; }

    /* assignment */
    register8& operator=(uint8_t value);
    register8& operator=(const address8& address);

    /* math ops */
    register8& operator++();
    register8& operator--();

    register8& operator+=(uint8_t value);
    register8& operator+=(const register8& reg);
    register8& operator+=(const address8& address);
    register8& operator-=(uint8_t value);
    register8& operator-=(const register8& reg);
    register8& operator-=(const address8& address);

    [[nodiscard]] uint16_t operator+(uint16_t value) const;
    [[nodiscard]] uint16_t operator+(const register8& reg) const;
    [[nodiscard]] uint16_t operator+(const address8& address) const;
    [[nodiscard]] uint16_t operator-(uint16_t value) const;
    [[nodiscard]] uint16_t operator-(const register8& reg) const;
    [[nodiscard]] uint16_t operator-(const address8& address) const;

    /* logical */
    register8& operator&=(uint8_t value);
    register8& operator&=(const register8& reg);
    register8& operator|=(uint8_t value);
    register8& operator|=(const register8& reg);
    register8& operator^=(uint8_t value);
    register8& operator^=(const register8& reg);

    register8 operator&(uint8_t value) const;
    register8 operator&(const register8& reg) const;
    register8 operator|(uint8_t value) const;
    register8 operator|(const register8& reg) const;
    register8 operator^(uint8_t value) const;
    register8 operator^(const register8& reg) const;

    register8 operator~() const;

    /* comparison */
    bool operator==(uint8_t value) const;
    bool operator==(const register8& reg) const;
    bool operator==(const address8& address) const;
    bool operator!=(uint8_t value) const;
    bool operator!=(const register8& reg) const;
    bool operator!=(const address8& address) const;

    bool operator>(uint8_t value) const;
    bool operator>(const register8& reg) const;
    bool operator>(const address8& address) const;
    bool operator<(uint8_t value) const;
    bool operator<(const register8& reg) const;
    bool operator<(const address8& address) const;
    bool operator>=(uint8_t value) const;
    bool operator>=(const register8& reg) const;
    bool operator>=(const address8& address) const;
    bool operator<=(uint8_t value) const;
    bool operator<=(const register8& reg) const;
    bool operator<=(const address8& address) const;

private:
    uint8_t bits = 0x00u;
};

}

#endif //GAMEBOY_REGISTER8_H
