#ifndef GAMEBOY_ALU_H
#define GAMEBOY_ALU_H

#include <cstdint>

#include "gameboy/util/observer.h"

namespace gameboy {

class cpu;
class register16;
class register8;

class alu {
public:
    static constexpr struct preserve_last_bit_t {} preserve_last_bit{};
    static constexpr struct reset_last_bit_t {} reset_last_bit{};

    explicit alu(observer<cpu> cpu_obs) noexcept
        : cpu_{cpu_obs} {}

    /* arithmetics */
    void increment(uint8_t& value) const noexcept;
    void increment(register8& reg) const noexcept;
    static void increment(register16& r) noexcept;

    void decrement(uint8_t& value) const noexcept;
    void decrement(register8& reg) const noexcept;
    static void decrement(register16& r) noexcept;

    void add(uint8_t value) const noexcept;
    void add(const register8& reg) const noexcept;
    void add(register16& r_left, const register16& r_right) const noexcept;
    void add_to_stack_pointer(int8_t immediate) const noexcept;
    void add_c(uint8_t value) const noexcept;
    void add_c(const register8& reg) const noexcept;

    void subtract(uint8_t value) const noexcept;
    void subtract(const register8& reg) const noexcept;
    void subtract_c(uint8_t value) const noexcept;
    void subtract_c(const register8& reg) const noexcept;

    /* logical */
    void logical_or(uint8_t value) const noexcept;
    void logical_or(const register8& reg) const noexcept;
    void logical_and(uint8_t value) const noexcept;
    void logical_and(const register8& reg) const noexcept;
    void logical_xor(uint8_t value) const noexcept;
    void logical_xor(const register8& reg) const noexcept;
    void logical_compare(uint8_t value) const noexcept;
    void logical_compare(const register8& reg) const noexcept;

    void complement() const noexcept;

    /* bitops */
    void test(uint8_t value, uint8_t bit) const noexcept;
    void test(const register8& reg, uint8_t bit) const noexcept;
    static void set(uint8_t& value, uint8_t bit) noexcept;
    void set(register8& reg, uint8_t bit) const noexcept;
    static void reset(uint8_t& value, uint8_t bit) noexcept;
    void reset(register8& reg, uint8_t bit) const noexcept;

    void rotate_left_acc() const noexcept;
    void rotate_right_acc() const noexcept;
    void rotate_left_c_acc() const noexcept;
    void rotate_right_c_acc() const noexcept;
    void rotate_left(uint8_t& value) const noexcept;
    void rotate_right(uint8_t& value) const noexcept;
    void rotate_left_c(uint8_t& value) const noexcept;
    void rotate_right_c(uint8_t& value) const noexcept;
    void rotate_left(register8& reg) const noexcept;
    void rotate_right(register8& reg) const noexcept;
    void rotate_left_c(register8& reg) const noexcept;
    void rotate_right_c(register8& reg) const noexcept;

    void shift_left(uint8_t& value) const noexcept;
    void shift_left(register8& reg) const noexcept;
    void shift_right(uint8_t& value, preserve_last_bit_t) const noexcept;
    void shift_right(register8& reg, preserve_last_bit_t tag) const noexcept;
    void shift_right(uint8_t& value, reset_last_bit_t) const noexcept;
    void shift_right(register8& reg, reset_last_bit_t tag) const noexcept;

    /* misc */
    void decimal_adjust() const noexcept;
    void swap(uint8_t& value) const noexcept;
    void swap(register8& reg) const noexcept;

private:
    observer<cpu> cpu_;
};

} // namespace gameboy

#endif //GAMEBOY_ALU_H
