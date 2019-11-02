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
    register16() noexcept = default;
    explicit register16(const uint16_t value) noexcept
        : high_{static_cast<uint8_t>(value >> 8u)},
          low_{static_cast<uint8_t>(value & 0xFFu)} {}

    [[nodiscard]] uint16_t value() const noexcept;

    /** @return most significant half of the register */
    [[nodiscard]] register8& high() noexcept { return high_; }

    /** @return most significant half of the register */
    [[nodiscard]] const register8& high() const noexcept { return high_; }

    /** @return least significant half of the register */
    [[nodiscard]] register8& low() noexcept { return low_; }

    /** @return least significant half of the register */
    [[nodiscard]] const register8& low() const noexcept { return low_; }

    /* assignment */
    register16& operator=(uint16_t val) noexcept;
    register16& operator=(const address16& address) noexcept;

    /* math ops */
    register16& operator++() noexcept;
    register16& operator--() noexcept;

    register16& operator+=(uint16_t val) noexcept;
    register16& operator+=(const register16& reg) noexcept;
    register16& operator+=(const address8& address) noexcept;

    register16& operator-=(uint16_t val) noexcept;

    [[nodiscard]] uint32_t operator+(uint32_t val) const noexcept;
    [[nodiscard]] uint32_t operator+(const register16& reg) const noexcept;

    [[nodiscard]] uint32_t operator-(uint32_t val) const noexcept;

    /* logical */
    register16 operator&(uint16_t val) const noexcept;
    register16 operator|(uint16_t val) const noexcept;
    register16 operator^(uint16_t val) const noexcept;
    register16 operator~() const noexcept;

private:
    register8 high_;
    register8 low_;
};

} // namespace gameboy

#endif //GAMEBOY_REGISTER16_H
