#ifndef GAMEBOY_REGISTER_H
#define GAMEBOY_REGISTER_H

#include <cstdint>

namespace gameboy {
    class Register;

    class HalfRegister {
        friend Register;

    public:
        void set(uint8_t index);
        void reset(uint8_t index);
        void flip(uint8_t index);
        [[nodiscard]] bool test(uint8_t index) const;

        [[nodiscard]] bool all() const;
        [[nodiscard]] bool any() const;
        [[nodiscard]] bool none() const;

        HalfRegister& operator=(uint8_t value);

        HalfRegister& operator++();
        HalfRegister& operator--();

        HalfRegister& operator+=(int8_t value);
        HalfRegister& operator-=(int8_t value);

        bool operator==(uint8_t value) const { return get_value() == value; }
        bool operator!=(uint8_t value) const { return get_value() != value; }
        bool operator==(const HalfRegister& r) const { return get_value() ==r.get_value(); }
        bool operator!=(const HalfRegister& r) const { return get_value() !=r.get_value(); }

    private:
        Register& reg;
        uint8_t offset;

        explicit HalfRegister(Register& reg, uint8_t offset)
            : reg(reg), offset(offset) {}

        [[nodiscard]] uint8_t get_value() const;
        void set_value(uint8_t value);
    };

    class Register {
        friend HalfRegister;

    public:
        Register() = default;
        explicit Register(uint16_t initial_value)
            : bits(initial_value) {}

        [[nodiscard]] const HalfRegister& get_high() const { return high; }
        [[nodiscard]] const HalfRegister& get_low() const { return low; }
        [[nodiscard]] HalfRegister& get_high() { return high; }
        [[nodiscard]] HalfRegister& get_low() { return low; }

        void set(uint8_t index);
        void reset(uint8_t index);
        void flip(uint8_t index);
        [[nodiscard]] bool test(uint8_t index) const;

        [[nodiscard]] bool all() const;
        [[nodiscard]] bool any() const;
        [[nodiscard]] bool none() const;

        Register& operator=(uint16_t value);

        Register& operator++();
        Register& operator--();

        Register& operator+=(int16_t value);
        Register& operator-=(int16_t value);

        bool operator==(uint16_t value) const { return bits == value; }
        bool operator!=(uint16_t value) const { return bits != value; }
        bool operator==(const Register& r) const { return bits == r.bits; }
        bool operator!=(const Register& r) const { return bits != r.bits; }

    private:
        uint16_t bits = 0x0000;
        HalfRegister high{*this, 8};
        HalfRegister low{*this, 0};
    };
}

#endif //GAMEBOY_REGISTER_H
