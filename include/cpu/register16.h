#ifndef GAMEBOY_REGISTER16_H
#define GAMEBOY_REGISTER16_H

#include <cstdint>
#include <cpu/register8.h>
#include <memory/addressfwd.h>

namespace gameboy {

/**
 * Represents a 16-bit register in the CPU
 */
class register16 {
public:
    register16() = default;
    explicit register16(const uint16_t value)
            :high((value >> 8u) & 0xFFu),
             low(value & 0xFFu) { }

    [[nodiscard]] uint16_t get_value() const;

    /** @return most significant half of the register */
    [[nodiscard]] register8& get_high() { return high; }

    /** @return most significant half of the register */
    [[nodiscard]] const register8& get_high() const { return high; }

    /** @return least significant half of the register */
    [[nodiscard]] register8& get_low() { return low; }

    /** @return least significant half of the register */
    [[nodiscard]] const register8& get_low() const { return low; }

    /* assignment */
    register16& operator=(uint16_t value);
    register16& operator=(const address16& address);

    /* math ops */
    register16& operator++();
    register16& operator--();

    register16& operator+=(uint16_t value);
    register16& operator+=(const register16& reg);
    register16& operator+=(const address16& address);
    register16& operator-=(uint16_t value);
    register16& operator-=(const register16& reg);
    register16& operator-=(const address16& address);

    [[nodiscard]] uint32_t operator+(uint32_t value) const;
    [[nodiscard]] uint32_t operator+(const register16& reg) const;
    [[nodiscard]] uint32_t operator+(const address16& address) const;
    [[nodiscard]] uint32_t operator-(uint32_t value) const;
    [[nodiscard]] uint32_t operator-(const register16& reg) const;
    [[nodiscard]] uint32_t operator-(const address16& address) const;

    /* logical */
    register16& operator&=(uint16_t value);
    register16& operator&=(const register16& reg);
    register16& operator|=(uint16_t value);
    register16& operator|=(const register16& reg);
    register16& operator^=(uint16_t value);
    register16& operator^=(const register16& reg);

    register16 operator&(uint16_t value) const;
    register16 operator&(const register16& reg) const;
    register16 operator|(uint16_t value) const;
    register16 operator|(const register16& reg) const;
    register16 operator^(uint16_t value) const;
    register16 operator^(const register16& reg) const;

    register16 operator~() const;

    /* comparison */
    bool operator==(uint16_t value) const;
    bool operator==(const register16& reg) const;
    bool operator==(const address16& address) const;
    bool operator!=(uint16_t value) const;
    bool operator!=(const register16& reg) const;
    bool operator!=(const address16& address) const;

    bool operator>(uint16_t value) const;
    bool operator>(const register16& reg) const;
    bool operator>(const address16& address) const;
    bool operator<(uint16_t value) const;
    bool operator<(const register16& reg) const;
    bool operator<(const address16& address) const;
    bool operator>=(uint16_t value) const;
    bool operator>=(const register16& reg) const;
    bool operator>=(const address16& address) const;
    bool operator<=(uint16_t value) const;
    bool operator<=(const register16& reg) const;
    bool operator<=(const address16& address) const;

private:
    register8 high;
    register8 low;
};

}

#endif //GAMEBOY_REGISTER16_H
