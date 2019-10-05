#ifndef GAMEBOY_ALU_H
#define GAMEBOY_ALU_H

#include <cstdint>

namespace gameboy::cpu {
    class CPU;
    class Register16;
    class Register8;

    class ALU {
    public:
        static constexpr struct PreserveLastBit { } preserve_last_bit{};
        static constexpr struct ResetLastBit { } reset_last_bit{};

        explicit ALU(CPU& cpu_ref)
                : cpu{cpu_ref} { }

        /* arithmetics */
        [[nodiscard]] uint8_t increment(uint8_t& value) const;
        [[nodiscard]] uint8_t increment(Register8& reg) const;
        [[nodiscard]] static uint8_t increment(Register16& r);

        [[nodiscard]] uint8_t decrement(uint8_t& value) const;
        [[nodiscard]] uint8_t decrement(Register8& reg) const;
        [[nodiscard]] static uint8_t decrement(Register16& r);

        [[nodiscard]] uint8_t add(uint8_t value) const;
        [[nodiscard]] uint8_t add(const Register8& reg) const;
        [[nodiscard]] uint8_t add(Register16& r_left, const Register16& r_right) const;
        [[nodiscard]] uint8_t add_to_stack_pointer(int8_t immediate) const;
        [[nodiscard]] uint8_t add_c(uint8_t value) const;
        [[nodiscard]] uint8_t add_c(const Register8& reg) const;

        [[nodiscard]] uint8_t subtract(uint8_t value) const;
        [[nodiscard]] uint8_t subtract(const Register8& reg) const;
        [[nodiscard]] uint8_t subtract_c(uint8_t value) const;
        [[nodiscard]] uint8_t subtract_c(const Register8& reg) const;

        /* logical */
        [[nodiscard]] uint8_t logical_or(uint8_t value) const;
        [[nodiscard]] uint8_t logical_or(const Register8& reg) const;
        [[nodiscard]] uint8_t logical_and(uint8_t value) const;
        [[nodiscard]] uint8_t logical_and(const Register8& reg) const;
        [[nodiscard]] uint8_t logical_xor(uint8_t value) const;
        [[nodiscard]] uint8_t logical_xor(const Register8& reg) const;
        [[nodiscard]] uint8_t logical_compare(uint8_t value) const;
        [[nodiscard]] uint8_t logical_compare(const Register8& reg) const;

        [[nodiscard]] uint8_t complement() const;

        /* bitops */
        [[nodiscard]] uint8_t bit_test(uint8_t value, uint8_t bit) const;
        [[nodiscard]] uint8_t bit_test(const Register8& reg, uint8_t bit) const;
        static uint8_t bit_set(uint8_t& value, uint8_t bit);
        uint8_t bit_set(Register8& reg, uint8_t bit) const;
        static uint8_t bit_reset(uint8_t& value, uint8_t bit);
        uint8_t bit_reset(Register8& reg, uint8_t bit) const;

        [[nodiscard]] uint8_t rotate_left_acc();
        [[nodiscard]] uint8_t rotate_right_acc();
        [[nodiscard]] uint8_t rotate_left_c_acc();
        [[nodiscard]] uint8_t rotate_right_c_acc();

        [[nodiscard]] uint8_t rotate_left(uint8_t& value) const;
        [[nodiscard]] uint8_t rotate_right(uint8_t& value) const;
        [[nodiscard]] uint8_t rotate_left_c(uint8_t& value) const;
        [[nodiscard]] uint8_t rotate_right_c(uint8_t& value) const;
        uint8_t rotate_left(Register8& reg) const;
        uint8_t rotate_right(Register8& reg) const;
        uint8_t rotate_left_c(Register8& reg) const;
        uint8_t rotate_right_c(Register8& reg) const;

        [[nodiscard]] uint8_t shift_left(uint8_t& value) const;
        [[nodiscard]] uint8_t shift_left(Register8& reg) const;
        [[nodiscard]] uint8_t shift_right(uint8_t& value, PreserveLastBit) const;
        [[nodiscard]] uint8_t shift_right(Register8& reg, PreserveLastBit tag) const;
        [[nodiscard]] uint8_t shift_right(uint8_t& value, ResetLastBit) const;
        [[nodiscard]] uint8_t shift_right(Register8& reg, ResetLastBit tag) const;

        /* misc */
        [[nodiscard]] uint8_t decimal_adjust() const;
        [[nodiscard]] uint8_t swap(uint8_t& value) const;
        [[nodiscard]] uint8_t swap(Register8& reg) const;

    private:
        CPU& cpu;
    };
}

#endif //GAMEBOY_ALU_H
