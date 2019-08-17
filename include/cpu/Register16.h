#ifndef GAMEBOY_REGISTER16_H
#define GAMEBOY_REGISTER16_H

#include <cstdint>
#include "Register8.h"

namespace gameboy::memory {
    class Address16;
}

namespace gameboy::cpu {
    class Register16 {
    public:
        Register16() = default;
        explicit Register16(uint16_t value)
            : high((value >> 8) & 0xFF),
              low(value & 0xFF) {}

        [[nodiscard]] uint16_t get_value() const;

        [[nodiscard]] Register8& get_high() { return high; }
        [[nodiscard]] const Register8& get_high() const { return high; }

        [[nodiscard]] Register8& get_low() { return low; }
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
