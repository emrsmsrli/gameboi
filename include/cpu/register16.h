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
        : high_((value >> 8u) & 0xFFu),
          low_(value & 0xFFu) {}

    [[nodiscard]] uint16_t value() const;

    /** @return most significant half of the register */
    [[nodiscard]] register8& high() { return high_; }

    /** @return most significant half of the register */
    [[nodiscard]] const register8& high() const { return high_; }

    /** @return least significant half of the register */
    [[nodiscard]] register8& low() { return low_; }

    /** @return least significant half of the register */
    [[nodiscard]] const register8& low() const { return low_; }

    /* assignment */
    register16& operator=(uint16_t val);
    register16& operator=(const address16& address);

    /* math ops */
    register16& operator++();
    register16& operator--();

    register16& operator+=(uint16_t val);
    register16& operator+=(const register16& reg);
    register16& operator+=(const address8& address);

    register16& operator-=(uint16_t val);

    [[nodiscard]] uint32_t operator+(uint32_t val) const;
    [[nodiscard]] uint32_t operator+(const register16& reg) const;

    [[nodiscard]] uint32_t operator-(uint32_t v) const;

    /* logical */
    register16 operator&(uint16_t val) const;
    register16 operator|(uint16_t val) const;
    register16 operator^(uint16_t val) const;
    register16 operator~() const;

private:
    register8 high_;
    register8 low_;
};

}

#endif //GAMEBOY_REGISTER16_H
