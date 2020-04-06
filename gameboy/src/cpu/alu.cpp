#include <magic_enum.hpp>

#include "gameboy/cpu/alu.h"
#include "gameboy/cpu/cpu.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

using namespace magic_enum::bitwise_operators;

void alu::add(const uint8_t value) const noexcept
{
    auto& acc = cpu_->a_f_.high();

    cpu_->reset_flag(cpu::flag::all);

    if(half_carry(acc.value(), value)) {
        cpu_->set_flag(cpu::flag::half_carry);
    }

    if(full_carry(acc.value(), value)) {
        cpu_->set_flag(cpu::flag::carry);
    }

    acc += value;

    if(acc == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }
}

void alu::add(const register8& reg) const noexcept
{
    add(reg.value());
}

void alu::add_c(const uint8_t value) const noexcept
{
    auto& acc = cpu_->a_f_.high();

    const auto carry = bit::from_bool(cpu_->test_flag(cpu::flag::carry));
    const auto result = acc + value + carry;

    cpu_->reset_flag(cpu::flag::all);

    if((acc & 0x0Fu) + (value & 0x0Fu) + carry > 0x0Fu) {
        cpu_->set_flag(cpu::flag::half_carry);
    }

    if(result > 0xFF) {
        cpu_->set_flag(cpu::flag::carry);
    }

    if((result & 0xFF) == 0x00) {
        cpu_->set_flag(cpu::flag::zero);
    }

    acc = static_cast<uint8_t>(result & 0xFFu);
}

void alu::add_c(const register8& reg) const noexcept
{
    add_c(reg.value());
}

void alu::subtract(const uint8_t value) const noexcept
{
    auto& acc = cpu_->a_f_.high();

    cpu_->reset_flag(cpu::flag::all);
    cpu_->set_flag(cpu::flag::negative);

    if(half_borrow(acc.value(), value)) {
        cpu_->set_flag(cpu::flag::half_carry);
    }

    if(full_borrow(acc.value(), value)) {
        cpu_->set_flag(cpu::flag::carry);
    }

    acc -= value;

    if(acc == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }
}

void alu::subtract(const register8& reg) const noexcept
{
    subtract(reg.value());
}

void alu::subtract_c(const uint8_t value) const noexcept
{
    auto& acc = cpu_->a_f_.high();

    const auto carry = static_cast<int8_t>(cpu_->test_flag(cpu::flag::carry));
    const auto result = acc.value() - value - carry;

    cpu_->reset_flag(cpu::flag::all);
    cpu_->set_flag(cpu::flag::negative);

    if((result & 0xFF) == 0x00) {
        cpu_->set_flag(cpu::flag::zero);
    }

    if(result < 0x00) {
        cpu_->set_flag(cpu::flag::carry);
    }

    if((acc.value() & 0x0F) - (value & 0x0F) - carry < 0x00) {
        cpu_->set_flag(cpu::flag::half_carry);
    }

    acc = static_cast<uint8_t>(result & 0xFF);
}

void alu::subtract_c(const register8& reg) const noexcept
{
    subtract_c(reg.value());
}

void alu::increment(uint8_t& value) const noexcept
{
    cpu_->reset_flag(cpu::flag::negative | cpu::flag::zero | cpu::flag::half_carry);

    if(half_carry(value, 1u)) {
        cpu_->set_flag(cpu::flag::half_carry);
    }

    ++value;

    if(value == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }
}

void alu::increment(register8& reg) const noexcept
{
    auto value = reg.value();
    increment(value);
    reg = value;
}

void alu::decrement(uint8_t& value) const noexcept
{
    cpu_->set_flag(cpu::flag::negative);
    cpu_->reset_flag(cpu::flag::half_carry | cpu::flag::zero);

    if(half_borrow(value, 1u)) {
        cpu_->set_flag(cpu::flag::half_carry);
    }

    --value;

    if(value == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }
}

void alu::decrement(register8& reg) const noexcept
{
    auto value = reg.value();
    decrement(value);
    reg = value;
}

void alu::logical_or(const uint8_t value) const noexcept
{
    auto& acc = cpu_->a_f_.high();
    acc |= value;

    cpu_->reset_flag(cpu::flag::all);
    if(acc == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }
}

void alu::logical_or(const register8& reg) const noexcept
{
    logical_or(reg.value());
}

void alu::logical_and(const uint8_t value) const noexcept
{
    auto& acc = cpu_->a_f_.high();
    acc &= value;

    cpu_->reset_flag(cpu::flag::all);
    cpu_->set_flag(cpu::flag::half_carry);
    if(acc == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }
}

void alu::logical_and(const register8& reg) const noexcept
{
    logical_and(reg.value());
}

void alu::logical_xor(const uint8_t value) const noexcept
{
    auto& acc = cpu_->a_f_.high();
    acc ^= value;

    cpu_->reset_flag(cpu::flag::all);
    if(acc == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }
}

void alu::logical_xor(const register8& reg) const noexcept
{
    logical_xor(reg.value());
}

void alu::logical_compare(const uint8_t value) const noexcept
{
    auto& acc = cpu_->a_f_.high();

    cpu_->reset_flag(cpu::flag::all);
    cpu_->set_flag(cpu::flag::negative);

    if(acc == value) {
        cpu_->set_flag(cpu::flag::zero);
    }

    if(half_borrow(acc.value(), value)) {
        cpu_->set_flag(cpu::flag::half_carry);
    }

    if(full_borrow(acc.value(), value)) {
        cpu_->set_flag(cpu::flag::carry);
    }
}

void alu::logical_compare(const register8& reg) const noexcept
{
    logical_compare(reg.value());
}

void alu::add(register16& r_left, const register16& r_right) const noexcept
{
    cpu_->reset_flag(cpu::flag::negative | cpu::flag::half_carry | cpu::flag::carry);

    if(half_carry(r_left.value(), r_right.value())) {
        cpu_->set_flag(cpu::flag::half_carry);
    }

    if(full_carry(r_left.value(), r_right.value())) {
        cpu_->set_flag(cpu::flag::carry);
    }

    r_left += r_right;
}

void alu::add_to_stack_pointer(const int8_t immediate) const noexcept
{
    auto& sp = cpu_->stack_pointer_;
    cpu_->reset_flag(cpu::flag::all);

    const auto result = sp.value() + immediate;

    if(mask::test(sp.value() ^ immediate ^ (result & 0xFFFF), 0x0100)) {
        cpu_->set_flag(cpu::flag::carry);
    }

    if(mask::test(sp.value() ^ immediate ^ (result & 0xFFFF), 0x0010)) {
        cpu_->set_flag(cpu::flag::half_carry);
    }

    cpu_->stack_pointer_ = static_cast<uint16_t>(result);
}

void alu::increment(register16& r) noexcept
{
    ++r;
}

void alu::decrement(register16& r) noexcept
{
    --r;
}

void alu::complement() const noexcept
{
    cpu_->a_f_.high() = ~cpu_->a_f_.high();
    cpu_->set_flag(cpu::flag::negative | cpu::flag::half_carry);
}

void alu::decimal_adjust() const noexcept
{
    int acc = cpu_->a_f_.high().value();

    if(cpu_->test_flag(cpu::flag::negative)) {
        if(cpu_->test_flag(cpu::flag::half_carry)) {
            acc = (acc - 0x06) & 0xFF;
        }

        if(cpu_->test_flag(cpu::flag::carry)) {
            acc -= 0x60;
        }
    } else {
        if(cpu_->test_flag(cpu::flag::half_carry) || (acc & 0x0F) > 0x09) {
            acc += 0x06;
        }

        if(cpu_->test_flag(cpu::flag::carry) || acc > 0x9F) {
            acc += 0x60;
        }
    }

    cpu_->reset_flag(cpu::flag::half_carry | cpu::flag::zero);

    if(mask::test(acc, 0x100)) {
        cpu_->set_flag(cpu::flag::carry);
    }

    acc &= 0xFF;
    if(acc == 0x00) {
        cpu_->set_flag(cpu::flag::zero);
    }

    cpu_->a_f_.high() = static_cast<uint8_t>(acc);
}

void alu::swap(uint8_t& value) const noexcept
{
    cpu_->reset_flag(cpu::flag::all);

    if(value == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    } else {
        value = (value << 0x4u) | (value >> 0x4u);
    }
}

void alu::swap(register8& reg) const noexcept
{
    auto value = reg.value();
    swap(value);
    reg = value;
}

void alu::test(const uint8_t value, const uint8_t bit) const noexcept
{
    cpu_->set_flag(cpu::flag::half_carry);
    cpu_->reset_flag(cpu::flag::negative | cpu::flag::zero);

    if(!bit::test(value, bit)) {
        cpu_->set_flag(cpu::flag::zero);
    }
}

void alu::test(const register8& reg, const uint8_t bit) const noexcept
{
    test(reg.value(), bit);
}

void alu::set(uint8_t& value, const uint8_t bit) noexcept
{
    value = bit::set(value, bit);
}

void alu::set(register8& reg, const uint8_t bit) const noexcept
{
    auto value = reg.value();
    set(value, bit);
    reg = value;
}

void alu::reset(uint8_t& value, const uint8_t bit) noexcept
{
    value = bit::reset(value, bit);
}

void alu::reset(register8& reg, const uint8_t bit) const noexcept
{
    auto value = reg.value();
    reset(value, bit);
    reg = value;
}

void alu::rotate_left_acc() const noexcept
{
    rotate_left(cpu_->a_f_.high());
    cpu_->reset_flag(cpu::flag::zero);
}

void alu::rotate_right_acc() const noexcept
{
    rotate_right(cpu_->a_f_.high());
    cpu_->reset_flag(cpu::flag::zero);
}

void alu::rotate_left_c_acc() const noexcept
{
    rotate_left_c(cpu_->a_f_.high());
    cpu_->reset_flag(cpu::flag::zero);
}

void alu::rotate_right_c_acc() const noexcept
{
    rotate_right_c(cpu_->a_f_.high());
    cpu_->reset_flag(cpu::flag::zero);
}

void alu::rotate_left(uint8_t& value) const noexcept
{
    const auto bit_7 = bit::extract(value, 7u);
    const auto carry = bit::from_bool(cpu_->test_flag(cpu::flag::carry));
    value = (value << 1u) | carry;

    cpu_->reset_flag(cpu::flag::all);
    if(bit_7 == 0x1u) {
        cpu_->set_flag(cpu::flag::carry);
    }

    if(value == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }
}

void alu::rotate_left(register8& reg) const noexcept
{
    auto value = reg.value();
    rotate_left(value);
    reg = value;
}

void alu::rotate_right(uint8_t& value) const noexcept
{
    const auto bit_0 = bit::extract(value, 0u);
    const auto carry = bit::from_bool(cpu_->test_flag(cpu::flag::carry));
    value = (value >> 1u) | (carry << 7u);

    cpu_->reset_flag(cpu::flag::all);
    if(bit_0 == 0x1u) {
        cpu_->set_flag(cpu::flag::carry);
    }

    if(value == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }
}

void alu::rotate_right(register8& reg) const noexcept
{
    auto value = reg.value();
    rotate_right(value);
    reg = value;
}

void alu::rotate_left_c(uint8_t& value) const noexcept
{
    const auto bit_7 = bit::extract(value, 7u);
    value = (value << 1u) | bit_7;

    cpu_->reset_flag(cpu::flag::all);
    if(bit_7 == 0x1u) {
        cpu_->set_flag(cpu::flag::carry);
    }

    if(value == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }
}

void alu::rotate_left_c(register8& reg) const noexcept
{
    auto value = reg.value();
    rotate_left_c(value);
    reg = value;
}

void alu::rotate_right_c(uint8_t& value) const noexcept
{
    const auto bit_0 = bit::extract(value, 0u);
    value = (value >> 1u) | (bit_0 << 7u);

    cpu_->reset_flag(cpu::flag::all);
    if(bit_0 == 0x1u) {
        cpu_->set_flag(cpu::flag::carry);
    }

    if(value == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }
}

void alu::rotate_right_c(register8& reg) const noexcept
{
    auto value = reg.value();
    rotate_right_c(value);
    reg = value;
}

void alu::shift_left(uint8_t& value) const noexcept
{
    const auto bit_7 = bit::extract(value, 7u);
    value <<= 1u;

    cpu_->reset_flag(cpu::flag::all);
    if(bit_7 == 0x1u) {
        cpu_->set_flag(cpu::flag::carry);
    }

    if(value == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }
}

void alu::shift_left(register8& reg) const noexcept
{
    auto value = reg.value();
    shift_left(value);
    reg = value;
}

void alu::shift_right(uint8_t& value, preserve_last_bit_t) const noexcept
{
    const auto bit_7 = value & 0x80u;
    const auto bit_0 = bit::extract(value, 0u);

    value = (value >> 1u) | bit_7;

    cpu_->reset_flag(cpu::flag::all);
    if(bit_0 == 0x1u) {
        cpu_->set_flag(cpu::flag::carry);
    }

    if(value == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }
}

void alu::shift_right(register8& reg, const preserve_last_bit_t tag) const noexcept
{
    auto value = reg.value();
    shift_right(value, tag);
    reg = value;
}

void alu::shift_right(uint8_t& value, reset_last_bit_t) const noexcept
{
    const auto bit_0 = bit::extract(value, 0u);

    value >>= 0x1u;

    cpu_->reset_flag(cpu::flag::all);
    if(bit_0 == 0x1u) {
        cpu_->set_flag(cpu::flag::carry);
    }

    if(value == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }
}

void alu::shift_right(register8& reg, const reset_last_bit_t tag) const noexcept
{
    auto value = reg.value();
    shift_right(value, tag);
    reg = value;
}

} // namespace gameboy
