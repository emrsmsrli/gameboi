#ifndef GAMEBOY_REGISTER8_H
#define GAMEBOY_REGISTER8_H

#include <cstdint>

namespace gameboy::cpu {
    class Register8 {
    public:
        Register8() = default;

        explicit Register8(uint8_t default_value)
                :bits(default_value) { }

        [[nodiscard]] uint16_t get_value() const { return bits; }

        // todo add address variants
        /* assignment */
        Register8& operator=(uint8_t value);
        Register8& operator=(const Register8& reg) = default;

        /* math ops */
        Register8& operator++();
        Register8& operator--();

        Register8& operator+=(uint8_t value);
        Register8& operator+=(const Register8& reg);
        Register8& operator-=(uint8_t value);
        Register8& operator-=(const Register8& reg);

        [[nodiscard]] uint16_t operator+(uint16_t value) const;
        [[nodiscard]] uint16_t operator+(const Register8& reg) const;
        [[nodiscard]] uint16_t operator-(uint16_t value) const;
        [[nodiscard]] uint16_t operator-(const Register8& reg) const;

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
        bool operator!=(uint8_t value) const;
        bool operator!=(const Register8& reg) const;

        bool operator>(uint8_t value) const;
        bool operator>(const Register8& reg) const;
        bool operator<(uint8_t value) const;
        bool operator<(const Register8& reg) const;
        bool operator>=(uint8_t value) const;
        bool operator>=(const Register8& reg) const;
        bool operator<=(uint8_t value) const;
        bool operator<=(const Register8& reg) const;

    private:
        uint8_t bits = 0x00;
    };
}

#endif //GAMEBOY_REGISTER8_H
