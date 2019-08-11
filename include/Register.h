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

        friend bool operator==(const HalfRegister& rl, const HalfRegister& rr);
        friend bool operator!=(const HalfRegister& rl, const HalfRegister& rr);

    private:
        Register& reg;
        uint8_t offset = 0u;

        explicit HalfRegister(Register& reg, uint8_t offset)
            : reg(reg), offset(offset) {}

        [[nodiscard]] uint8_t get() const;
    };

    class Register {
        friend HalfRegister;

    public:
        Register() = default;

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

        Register& operator+=(uint16_t value);
        Register& operator-=(uint16_t value);

        friend bool operator==(const Register& rl, const Register& rr) { return rl.bits == rr.bits; }
        friend bool operator!=(const Register& rl, const Register& rr) { return !(rl == rr); }

    private:
        uint16_t bits = 0;
        HalfRegister high{*this, 8};
        HalfRegister low{*this, 0};
    };
}

#endif //GAMEBOY_REGISTER_H
