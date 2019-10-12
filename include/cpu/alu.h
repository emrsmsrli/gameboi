#ifndef GAMEBOY_ALU_H
#define GAMEBOY_ALU_H

#include <cstdint>

#include <util/observer.h>

namespace gameboy {

class cpu;
class register16;
class register8;

class alu {
public:
    static constexpr struct preserve_last_bit_t {} preserve_last_bit{};
    static constexpr struct reset_last_bit_t {} reset_last_bit{};

    explicit alu(observer<cpu> cpu_obs)
        : cpu_{cpu_obs} {}

    /* arithmetics */
    [[nodiscard]] uint8_t increment(uint8_t& value) const;
    [[nodiscard]] uint8_t increment(register8& reg) const;
    [[nodiscard]] static uint8_t increment(register16& r);

    [[nodiscard]] uint8_t decrement(uint8_t& value) const;
    [[nodiscard]] uint8_t decrement(register8& reg) const;
    [[nodiscard]] static uint8_t decrement(register16& r);

    [[nodiscard]] uint8_t add(uint8_t value) const;
    [[nodiscard]] uint8_t add(const register8& reg) const;
    [[nodiscard]] uint8_t add(register16& r_left, const register16& r_right) const;
    [[nodiscard]] uint8_t add_to_stack_pointer(int8_t immediate) const;
    [[nodiscard]] uint8_t add_c(uint8_t value) const;
    [[nodiscard]] uint8_t add_c(const register8& reg) const;

    [[nodiscard]] uint8_t subtract(uint8_t value) const;
    [[nodiscard]] uint8_t subtract(const register8& reg) const;
    [[nodiscard]] uint8_t subtract_c(uint8_t value) const;
    [[nodiscard]] uint8_t subtract_c(const register8& reg) const;

    /* logical */
    [[nodiscard]] uint8_t logical_or(uint8_t value) const;
    [[nodiscard]] uint8_t logical_or(const register8& reg) const;
    [[nodiscard]] uint8_t logical_and(uint8_t value) const;
    [[nodiscard]] uint8_t logical_and(const register8& reg) const;
    [[nodiscard]] uint8_t logical_xor(uint8_t value) const;
    [[nodiscard]] uint8_t logical_xor(const register8& reg) const;
    [[nodiscard]] uint8_t logical_compare(uint8_t value) const;
    [[nodiscard]] uint8_t logical_compare(const register8& reg) const;

    [[nodiscard]] uint8_t complement() const;

    /* bitops */
    [[nodiscard]] uint8_t bit_test(uint8_t value, uint8_t bit) const;
    [[nodiscard]] uint8_t bit_test(const register8& reg, uint8_t bit) const;
    static uint8_t bit_set(uint8_t& value, uint8_t bit);
    uint8_t bit_set(register8& reg, uint8_t bit) const;
    static uint8_t bit_reset(uint8_t& value, uint8_t bit);
    uint8_t bit_reset(register8& reg, uint8_t bit) const;

    [[nodiscard]] uint8_t rotate_left_acc() const;
    [[nodiscard]] uint8_t rotate_right_acc() const;
    [[nodiscard]] uint8_t rotate_left_c_acc() const;
    [[nodiscard]] uint8_t rotate_right_c_acc() const;
    [[nodiscard]] uint8_t rotate_left(uint8_t& value) const;
    [[nodiscard]] uint8_t rotate_right(uint8_t& value) const;
    [[nodiscard]] uint8_t rotate_left_c(uint8_t& value) const;
    [[nodiscard]] uint8_t rotate_right_c(uint8_t& value) const;
    uint8_t rotate_left(register8& reg) const;
    uint8_t rotate_right(register8& reg) const;
    uint8_t rotate_left_c(register8& reg) const;
    uint8_t rotate_right_c(register8& reg) const;

    [[nodiscard]] uint8_t shift_left(uint8_t& value) const;
    [[nodiscard]] uint8_t shift_left(register8& reg) const;
    [[nodiscard]] uint8_t shift_right(uint8_t& value, preserve_last_bit_t) const;
    [[nodiscard]] uint8_t shift_right(register8& reg, preserve_last_bit_t tag) const;
    [[nodiscard]] uint8_t shift_right(uint8_t& value, reset_last_bit_t) const;
    [[nodiscard]] uint8_t shift_right(register8& reg, reset_last_bit_t tag) const;

    /* misc */
    [[nodiscard]] uint8_t decimal_adjust() const;
    [[nodiscard]] uint8_t swap(uint8_t& value) const;
    [[nodiscard]] uint8_t swap(register8& reg) const;

private:
    observer<cpu> cpu_;
};

}

#endif //GAMEBOY_ALU_H