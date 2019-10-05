#ifndef GAMEBOY_REGISTER8_H
#define GAMEBOY_REGISTER8_H

#include <cstdint>
#include <memory/AddressFwd.h>

namespace gameboy::cpu {
    /**
     * Represents an 8-bit register in the CPU
     */
    class Register8 {
    public:
        Register8() = default;

        explicit Register8(const uint8_t default_value)
                : bits(default_value) { }

        [[nodiscard]] uint8_t get_value() const { return bits; }

        /* assignment */
        Register8& operator=(uint8_t value);
        Register8& operator=(const memory::Address8& address);

        /* math ops */
        Register8& operator++();
        Register8& operator--();

        Register8& operator+=(uint8_t value);
        Register8& operator+=(const Register8& reg);
        Register8& operator+=(const memory::Address8& address);
        Register8& operator-=(uint8_t value);
        Register8& operator-=(const Register8& reg);
        Register8& operator-=(const memory::Address8& address);

        [[nodiscard]] uint16_t operator+(uint16_t value) const;
        [[nodiscard]] uint16_t operator+(const Register8& reg) const;
        [[nodiscard]] uint16_t operator+(const memory::Address8& address) const;
        [[nodiscard]] uint16_t operator-(uint16_t value) const;
        [[nodiscard]] uint16_t operator-(const Register8& reg) const;
        [[nodiscard]] uint16_t operator-(const memory::Address8& address) const;

        /* logical */
        Register8& operator&=(uint8_t value);
        Register8& operator&=(const Register8& reg);
        Register8& operator|=(uint8_t value);
        Register8& operator|=(const Register8& reg);
        Register8& operator^=(uint8_t value);
        Register8& operator^=(const Register8& reg);

        Register8 operator&(uint8_t value) const;
        Register8 operator&(const Register8& reg) const;
        Register8 operator|(uint8_t value) const;
        Register8 operator|(const Register8& reg) const;
        Register8 operator^(uint8_t value) const;
        Register8 operator^(const Register8& reg) const;

        Register8 operator~() const;

        /* comparison */
        bool operator==(uint8_t value) const;
        bool operator==(const Register8& reg) const;
        bool operator==(const memory::Address8& address) const;
        bool operator!=(uint8_t value) const;
        bool operator!=(const Register8& reg) const;
        bool operator!=(const memory::Address8& address) const;

        bool operator>(uint8_t value) const;
        bool operator>(const Register8& reg) const;
        bool operator>(const memory::Address8& address) const;
        bool operator<(uint8_t value) const;
        bool operator<(const Register8& reg) const;
        bool operator<(const memory::Address8& address) const;
        bool operator>=(uint8_t value) const;
        bool operator>=(const Register8& reg) const;
        bool operator>=(const memory::Address8& address) const;
        bool operator<=(uint8_t value) const;
        bool operator<=(const Register8& reg) const;
        bool operator<=(const memory::Address8& address) const;

    private:
        uint8_t bits = 0x00u;
    };
}

#endif //GAMEBOY_REGISTER8_H
