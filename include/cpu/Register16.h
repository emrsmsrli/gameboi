#ifndef GAMEBOY_REGISTER16_H
#define GAMEBOY_REGISTER16_H

#include <cstdint>
#include "Register8.h"
#include "memory/AddressFwd.h"

namespace gameboy::cpu {
    /**
     * Represents a 16-bit register in the CPU
     */
    class Register16 {
    public:
        Register16() = default;
        explicit Register16(uint16_t value)
            : high((value >> 8u) & 0xFFu),
              low(value & 0xFFu) {}

        [[nodiscard]] uint16_t get_value() const;

        /** @return most significant half of the register */
        [[nodiscard]] Register8& get_high() { return high; }

        /** @return most significant half of the register */
        [[nodiscard]] const Register8& get_high() const { return high; }

        /** @return least significant half of the register */
        [[nodiscard]] Register8& get_low() { return low; }

        /** @return least significant half of the register */
        [[nodiscard]] const Register8& get_low() const { return low; }

        /* assignment */
        Register16& operator=(uint16_t value);
        Register16& operator=(const Register16& reg) = default;
        Register16& operator=(const memory::Address16& address);

        /* math ops */
        Register16& operator++();
        Register16& operator--();

        Register16& operator+=(uint16_t value);
        Register16& operator+=(const Register16& reg);
        Register16& operator+=(const memory::Address16& address);
        Register16& operator-=(uint16_t value);
        Register16& operator-=(const Register16& reg);
        Register16& operator-=(const memory::Address16& address);

        [[nodiscard]] uint32_t operator+(uint32_t value) const;
        [[nodiscard]] uint32_t operator+(const Register16& reg) const;
        [[nodiscard]] uint32_t operator+(const memory::Address16& address) const;
        [[nodiscard]] uint32_t operator-(uint32_t value) const;
        [[nodiscard]] uint32_t operator-(const Register16& reg) const;
        [[nodiscard]] uint32_t operator-(const memory::Address16& address) const;

        /* logical */
        Register16& operator&=(uint16_t value);
        Register16& operator&=(const Register16& reg);
        Register16& operator|=(uint16_t value);
        Register16& operator|=(const Register16& reg);
        Register16& operator^=(uint16_t value);
        Register16& operator^=(const Register16& reg);

        Register16 operator&(uint16_t value) const;
        Register16 operator&(const Register16& reg) const;
        Register16 operator|(uint16_t value) const;
        Register16 operator|(const Register16& reg) const;
        Register16 operator^(uint16_t value) const;
        Register16 operator^(const Register16& reg) const;

        Register16 operator~() const;

        /* comparison */
        bool operator==(uint16_t value) const;
        bool operator==(const Register16& reg) const;
        bool operator==(const memory::Address16& address) const;
        bool operator!=(uint16_t value) const;
        bool operator!=(const Register16& reg) const;
        bool operator!=(const memory::Address16& address) const;

        bool operator>(uint16_t value) const;
        bool operator>(const Register16& reg) const;
        bool operator>(const memory::Address16& address) const;
        bool operator<(uint16_t value) const;
        bool operator<(const Register16& reg) const;
        bool operator<(const memory::Address16& address) const;
        bool operator>=(uint16_t value) const;
        bool operator>=(const Register16& reg) const;
        bool operator>=(const memory::Address16& address) const;
        bool operator<=(uint16_t value) const;
        bool operator<=(const Register16& reg) const;
        bool operator<=(const memory::Address16& address) const;

    private:
        Register8 high;
        Register8 low;
    };
}

#endif //GAMEBOY_REGISTER16_H
