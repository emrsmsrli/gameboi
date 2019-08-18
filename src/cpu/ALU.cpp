
#include "cpu/ALU.h"
#include "cpu/CPU.h"
#include "util/Math.h"

uint8_t gameboy::cpu::ALU::add(uint8_t value)
{
    auto& acc = cpu.a_f.get_high();

    cpu.reset_flag(Flag::all);
    if(((acc + value) & 0xFF) == 0x00) {
        cpu.set_flag(Flag::zero);
    }

    if((acc & 0x0F) + (value & 0x0F) > 0x0F) {
        cpu.set_flag(Flag::half_carry);
    }

    if(acc + value > 0xFF) {
        cpu.set_flag(Flag::carry);
    }

    acc += value;
    return 4;
}

uint8_t gameboy::cpu::ALU::add(const gameboy::cpu::Register8& reg)
{
    return add(reg.get_value());
}

uint8_t gameboy::cpu::ALU::add_c(uint8_t value)
{
    auto& acc = cpu.a_f.get_high();

    const uint8_t carry = cpu.test_flag(Flag::carry) ? 0x01 : 0x00;
    const uint16_t result = acc + value + carry;

    cpu.reset_flag(Flag::all);

    if((result & 0xFF) == 0x00) {
        cpu.set_flag(Flag::zero);
    }

    if((acc & 0x0F) + (value & 0x0F) + carry > 0x0F) {
        cpu.set_flag(Flag::half_carry);
    }

    if(result > 0xFF) {
        cpu.set_flag(Flag::carry);
    }

    acc = result & 0xFF;
    return 4;
}

uint8_t gameboy::cpu::ALU::add_c(const gameboy::cpu::Register8& reg)
{
    return add_c(reg.get_value());
}

uint8_t gameboy::cpu::ALU::subtract(uint8_t value)
{
    auto& acc = cpu.a_f.get_high();

    cpu.reset_flag(Flag::all);
    cpu.set_flag(Flag::subtract);

    if(acc - value == 0x00) {
        cpu.set_flag(Flag::zero);
    }

    if((acc & 0x0F) < (value & 0x0F)) {
        cpu.set_flag(Flag::half_carry);
    }

    if(acc < value) {
        cpu.set_flag(Flag::carry);
    }

    acc -= value;
    return 4;
}

uint8_t gameboy::cpu::ALU::subtract(const gameboy::cpu::Register8& reg)
{
    return subtract(reg.get_value());
}

uint8_t gameboy::cpu::ALU::subtract_c(uint8_t value)
{
    auto& acc = cpu.a_f.get_high();

    const int16_t carry = cpu.test_flag(Flag::carry) ? 0x01 : 0x00;
    const int16_t result = acc - value - carry;

    cpu.reset_flag(Flag::all);
    cpu.set_flag(Flag::subtract);

    if(result == 0x00) {
        cpu.set_flag(Flag::zero);
    }

    if(result < 0x00) {
        cpu.set_flag(Flag::zero);
    }

    if((acc & 0x0F) - (value & 0x0F) - carry < 0x0F) {
        cpu.set_flag(Flag::half_carry);
    }

    acc = result;
    return 4;
}

uint8_t gameboy::cpu::ALU::subtract_c(const gameboy::cpu::Register8& reg)
{
    return subtract_c(reg.get_value());
}

uint8_t gameboy::cpu::ALU::increment(uint8_t& value)
{
    ++value;

    cpu.reset_flag(Flag::subtract);
    if(value == 0x00) {
        cpu.set_flag(Flag::zero);
    } else {
        cpu.reset_flag(Flag::zero);
    }

    if((value & 0x0F) != 0x00) {
        cpu.reset_flag(Flag::half_carry);
    } else {
        cpu.set_flag(Flag::half_carry);
    }

    return 4;
}

uint8_t gameboy::cpu::ALU::increment(gameboy::cpu::Register8& reg) {
    auto value = reg.get_value();

    const auto cycles = increment(value);
    reg = value;

    return cycles;
}

uint8_t gameboy::cpu::ALU::decrement(uint8_t& value)
{
    --value;

    cpu.set_flag(Flag::subtract);
    if(value == 0x00) {
        cpu.set_flag(Flag::zero);
    } else {
        cpu.reset_flag(Flag::zero);
    }

    if((value & 0x0F) == 0x0F) {
        cpu.set_flag(Flag::half_carry);
    } else {
        cpu.reset_flag(Flag::half_carry);
    }

    return 4;
}

uint8_t gameboy::cpu::ALU::decrement(gameboy::cpu::Register8& reg) {
    auto value = reg.get_value();

    const auto cycles = decrement(value);
    reg = value;

    return cycles;
}

uint8_t gameboy::cpu::ALU::logical_or(uint8_t value)
{
    auto& acc = cpu.a_f.get_high();
    acc = acc | value;

    cpu.reset_flag(Flag::all);
    if(acc == 0x00) {
        cpu.set_flag(Flag::zero);
    }

    return 4;
}

uint8_t gameboy::cpu::ALU::logical_or(const gameboy::cpu::Register8& reg)
{
    return logical_or(reg.get_value());
}

uint8_t gameboy::cpu::ALU::logical_and(uint8_t value)
{
    auto& acc = cpu.a_f.get_high();
    acc = acc & value;

    cpu.reset_flag(Flag::all);
    cpu.set_flag(Flag::half_carry);
    if(acc == 0x00) {
        cpu.set_flag(Flag::zero);
    }

    return 4;
}

uint8_t gameboy::cpu::ALU::logical_and(const gameboy::cpu::Register8& reg)
{
    return logical_and(reg.get_value());
}

uint8_t gameboy::cpu::ALU::logical_xor(uint8_t value)
{
    auto& acc = cpu.a_f.get_high();
    acc = acc ^ value;

    cpu.reset_flag(Flag::all);
    if(acc == 0x00) {
        cpu.set_flag(Flag::zero);
    }

    return 4;
}

uint8_t gameboy::cpu::ALU::logical_xor(const gameboy::cpu::Register8& reg)
{
    return logical_xor(reg.get_value());
}

uint8_t gameboy::cpu::ALU::logical_compare(uint8_t value)
{
    auto& acc = cpu.a_f.get_high();

    cpu.reset_flag(Flag::all);
    cpu.set_flag(Flag::subtract);

    if(acc == value) {
        cpu.set_flag(Flag::zero);
    }

    if((acc & 0x0F) < (value & 0x0F)) {
        cpu.set_flag(Flag::half_carry);
    }

    if(acc < value) {
        cpu.set_flag(Flag::carry);
    }

    return 4;
}

uint8_t gameboy::cpu::ALU::logical_compare(const gameboy::cpu::Register8& reg)
{
    return logical_compare(reg.get_value());
}

uint8_t gameboy::cpu::ALU::add(gameboy::cpu::Register16& r_left, const gameboy::cpu::Register16& right_side)
{
    cpu.reset_flag(Flag::subtract);

    if ((r_left & 0x0FFF) + (right_side & 0x0FFF) > 0x0FFF) {
        cpu.set_flag(Flag::half_carry);
    } else {
        cpu.reset_flag(Flag::half_carry);
    }

    if (r_left + right_side > 0xFFFF) {
        cpu.set_flag(Flag::carry);
    } else {
        cpu.reset_flag(Flag::carry);
    }

    r_left += right_side;
    return 8;
}

uint8_t gameboy::cpu::ALU::add_to_stack_pointer(int8_t immidiate)
{
    cpu.reset_flag(Flag::all);

    if((cpu.stack_pointer & 0x0F) + (immidiate & 0x0F) > 0x0F) {
        cpu.set_flag(Flag::half_carry);
    }

    if((cpu.stack_pointer & 0xFF) + (immidiate & 0xFF) > 0xFF) {
        cpu.set_flag(Flag::carry);
    }

    cpu.stack_pointer += immidiate;

    return 16;
}

uint8_t gameboy::cpu::ALU::increment(gameboy::cpu::Register16& r) const
{
    r += 1;
    return 8;
}

uint8_t gameboy::cpu::ALU::decrement(gameboy::cpu::Register16& r) const
{
    r -= 1;
    return 8;
}

uint8_t gameboy::cpu::ALU::complement()
{
    cpu.a_f.get_high() = ~cpu.a_f.get_high();
    cpu.set_flag(Flag::subtract);
    cpu.set_flag(Flag::half_carry);
    return 4;
}

uint8_t gameboy::cpu::ALU::decimal_adjust()
{
    uint16_t acc = cpu.a_f.get_high().get_value();

    if(cpu.test_flag(Flag::subtract)) {
        if(cpu.test_flag(Flag::half_carry)) {
            acc = (acc - 0x06) & 0xFF;
        }

        if(cpu.test_flag(Flag::carry)) {
            acc -= 0x60;
        }
    } else {
        if(cpu.test_flag(Flag::half_carry) || (acc & 0x0F) > 0x09) {
            acc += 0x06;
        }

        if(cpu.test_flag(Flag::carry) || acc > 0x9F) {
            acc += 0x60;
        }
    }

    cpu.reset_flag(Flag::half_carry);
    cpu.reset_flag(Flag::zero);

    if((acc & 0x100) == 0x100) {
        cpu.set_flag(Flag::carry);
    }

    acc &= 0xFF;

    if(acc == 0x00) {
        cpu.set_flag(Flag::zero);
    }

    cpu.a_f.get_high() = acc;

    return 4;
}

uint8_t gameboy::cpu::ALU::swap(uint8_t& value)
{
    cpu.reset_flag(Flag::all);

    if(value == 0x00) {
        cpu.set_flag(Flag::zero);
    } else {
        value = (value << 4) | (value >> 4);
    }

    return 8;
}

uint8_t gameboy::cpu::ALU::swap(gameboy::cpu::Register8& reg)
{
    auto value = reg.get_value();

    const auto cycles = swap(value);
    reg = value;

    return cycles;
}

uint8_t gameboy::cpu::ALU::bit_test(uint8_t value, uint8_t bit)
{
    if(math::bit_test(value, bit)) {
        cpu.set_flag(Flag::zero);
    } else {
        cpu.reset_flag(Flag::zero);
    }

    cpu.set_flag(Flag::half_carry);
    cpu.reset_flag(Flag::subtract);
    return 8;
}

uint8_t gameboy::cpu::ALU::bit_test(const gameboy::cpu::Register8& reg, uint8_t bit)
{
    return bit_test(reg.get_value(), bit);
}

uint8_t gameboy::cpu::ALU::bit_set(uint8_t& value, uint8_t bit) const
{
    math::bit_set(value, bit);
    return 8;
}

uint8_t gameboy::cpu::ALU::bit_set(gameboy::cpu::Register8& reg, uint8_t bit) const
{
    auto value = reg.get_value();
    const auto cycles = bit_set(value, bit);
    reg = value;
    return cycles;
}

uint8_t gameboy::cpu::ALU::bit_reset(uint8_t& value, uint8_t bit) const
{
    math::bit_reset(value, bit);
    return 8;
}

uint8_t gameboy::cpu::ALU::bit_reset(gameboy::cpu::Register8& reg, uint8_t bit) const
{
    auto value = reg.get_value();
    const auto cycles = bit_reset(value, bit);
    reg = value;
    return cycles;
}

uint8_t gameboy::cpu::ALU::rotate_left_acc()
{
    rotate_left(cpu.a_f.get_high());
    cpu.reset_flag(Flag::zero);
    return 4;
}

uint8_t gameboy::cpu::ALU::rotate_right_acc()
{
    rotate_right(cpu.a_f.get_high());
    cpu.reset_flag(Flag::zero);
    return 4;
}

uint8_t gameboy::cpu::ALU::rotate_left_c_acc()
{
    rotate_left_c(cpu.a_f.get_high());
    cpu.reset_flag(Flag::zero);
    return 4;
}

uint8_t gameboy::cpu::ALU::rotate_right_c_acc()
{
    rotate_right_c(cpu.a_f.get_high());
    cpu.reset_flag(Flag::zero);
    return 4;
}

uint8_t gameboy::cpu::ALU::rotate_left(uint8_t& value)
{
    const auto msb = value & 0x80;
    value <<= 0x1;

    if(cpu.test_flag(Flag::carry)) {
        bit_set(value, 0);
    }  else {
        bit_reset(value, 0);
    }

    cpu.reset_flag(Flag::all);
    if(msb != 0x00) {
        cpu.set_flag(Flag::carry);
    }

    if(value == 0x00) {
        cpu.set_flag(Flag::zero);
    }

    return 8;
}

uint8_t gameboy::cpu::ALU::rotate_left(gameboy::cpu::Register8& reg)
{
    auto value = reg.get_value();
    const auto cycles = rotate_left(value);
    reg = value;
    return cycles;
}

uint8_t gameboy::cpu::ALU::rotate_right(uint8_t& value)
{
    const auto lsb = value & 0x01;
    value >>= 0x1;

    if(cpu.test_flag(Flag::carry)) {
        bit_set(value, 7);
    }
    else {
        bit_reset(value, 7);
    }

    cpu.reset_flag(Flag::all);
    if(lsb == 0x01) {
        cpu.set_flag(Flag::carry);
    }

    if(value == 0x00) {
        cpu.set_flag(Flag::zero);
    }

    return 8;
}

uint8_t gameboy::cpu::ALU::rotate_right(gameboy::cpu::Register8& reg)
{
    auto value = reg.get_value();
    const auto cycles = rotate_right(value);
    reg = value;
    return cycles;
}

uint8_t gameboy::cpu::ALU::rotate_left_c(uint8_t& value)
{
    const auto msb = value & 0x80;
    value <<= 0x1;

    cpu.reset_flag(Flag::all);
    if(msb != 0x00) {
        cpu.set_flag(Flag::carry);
        bit_set(value, 0);
    } else {
        bit_reset(value, 0);
    }

    if(value == 0x00) {
        cpu.set_flag(Flag::zero);
    }

    return 8;
}

uint8_t gameboy::cpu::ALU::rotate_left_c(gameboy::cpu::Register8& reg)
{
    auto value = reg.get_value();
    const auto cycles = rotate_left_c(value);
    reg = value;
    return cycles;
}

uint8_t gameboy::cpu::ALU::rotate_right_c(uint8_t& value)
{
    const auto lsb = value & 0x01;
    value >>= 0x1;

    cpu.reset_flag(Flag::all);
    if(lsb != 0x00) {
        cpu.set_flag(Flag::carry);
        bit_set(value, 7);
    } else {
        bit_reset(value, 7);
    }

    if(value == 0x00) {
        cpu.set_flag(Flag::zero);
    }

    return 8;
}

uint8_t gameboy::cpu::ALU::rotate_right_c(gameboy::cpu::Register8& reg)
{
    auto value = reg.get_value();
    const auto cycles = rotate_right_c(value);
    reg = value;
    return cycles;
}

uint8_t gameboy::cpu::ALU::shift_left(uint8_t& value)
{
    const auto msb = value & 0x80;
    value <<= 0x1;

    cpu.reset_flag(Flag::all);
    if(msb != 0x00) {
        cpu.set_flag(Flag::carry);
    }

    if(value == 0x00) {
        cpu.set_flag(Flag::zero);
    }

    return 8;
}

uint8_t gameboy::cpu::ALU::shift_left(gameboy::cpu::Register8& reg)
{
    auto value = reg.get_value();
    const auto cycles = shift_left(value);
    reg = value;
    return cycles;
}

uint8_t gameboy::cpu::ALU::shift_right(uint8_t& value, gameboy::cpu::tag::PreserveLastBit)
{
    const auto msb = value & 0x80;
    const auto lsb = value & 0x01;

    value >>= 0x1;

    if(msb != 0x00) {
        bit_set(value, 7);
    } else {
        bit_reset(value, 7);
    }

    cpu.reset_flag(Flag::all);
    if(lsb == 0x01) {
        cpu.set_flag(Flag::carry);
    }

    if(value == 0x00) {
        cpu.set_flag(Flag::zero);
    }

    return 8;
}

uint8_t gameboy::cpu::ALU::shift_right(gameboy::cpu::Register8& reg, gameboy::cpu::tag::PreserveLastBit tag)
{
    auto value = reg.get_value();
    const auto cycles = shift_right(value, tag);
    reg = value;
    return cycles;
}

uint8_t gameboy::cpu::ALU::shift_right(uint8_t& value, gameboy::cpu::tag::ResetLastBit)
{
    const auto lsb = value & 0x01;

    value >>= 0x1;
    bit_reset(value, 7);

    cpu.reset_flag(Flag::all);
    if(lsb == 0x01) {
        cpu.set_flag(Flag::carry);
    }

    if(value == 0x00) {
        cpu.set_flag(Flag::zero);
    }

    return 8;
}

uint8_t gameboy::cpu::ALU::shift_right(gameboy::cpu::Register8& reg, gameboy::cpu::tag::ResetLastBit tag)
{
    auto value = reg.get_value();
    const auto cycles = shift_right(value, tag);
    reg = value;
    return cycles;
}
