#include <cpu/ALU.h>
#include <cpu/CPU.h>
#include <util/Math.h>

uint8_t gameboy::cpu::ALU::add(const uint8_t value) const
{
    auto& acc = cpu.a_f.get_high();

    cpu.reset_flag(Flag::all);
    if(((acc + value) & 0xFFu) == 0x00u) {
        cpu.set_flag(Flag::zero);
    }

    if((acc & 0x0Fu) + (value & 0x0Fu) > 0x0Fu) {
        cpu.set_flag(Flag::half_carry);
    }

    if(acc + value > 0xFFu) {
        cpu.set_flag(Flag::carry);
    }

    acc += value;
    return 4u;
}

uint8_t gameboy::cpu::ALU::add(const gameboy::cpu::Register8& reg) const
{
    return add(reg.get_value());
}

uint8_t gameboy::cpu::ALU::add_c(const uint8_t value) const
{
    auto& acc = cpu.a_f.get_high();

    const uint8_t carry = cpu.test_flag(Flag::carry) ? 0x01u : 0x00u;
    const uint16_t result = acc + value + carry;

    cpu.reset_flag(Flag::all);

    if((result & 0xFFu) == 0x00u) {
        cpu.set_flag(Flag::zero);
    }

    if((acc & 0x0Fu) + (value & 0x0Fu) + carry > 0x0Fu) {
        cpu.set_flag(Flag::half_carry);
    }

    if(result > 0xFFu) {
        cpu.set_flag(Flag::carry);
    }

    acc = result & 0xFFu;
    return 4u;
}

uint8_t gameboy::cpu::ALU::add_c(const gameboy::cpu::Register8& reg) const
{
    return add_c(reg.get_value());
}

uint8_t gameboy::cpu::ALU::subtract(const uint8_t value) const
{
    auto& acc = cpu.a_f.get_high();

    cpu.reset_flag(Flag::all);
    cpu.set_flag(Flag::subtract);

    if(acc - value == 0x00u) {
        cpu.set_flag(Flag::zero);
    }

    if((acc & 0x0Fu) < (value & 0x0Fu)) {
        cpu.set_flag(Flag::half_carry);
    }

    if(acc < value) {
        cpu.set_flag(Flag::carry);
    }

    acc -= value;
    return 4u;
}

uint8_t gameboy::cpu::ALU::subtract(const gameboy::cpu::Register8& reg) const
{
    return subtract(reg.get_value());
}

uint8_t gameboy::cpu::ALU::subtract_c(const uint8_t value) const
{
    auto& acc = cpu.a_f.get_high();

    const auto carry = cpu.test_flag(Flag::carry) ? 0x01u : 0x00u;
    const auto result = acc - value - carry;

    cpu.reset_flag(Flag::all);
    cpu.set_flag(Flag::subtract);

    if(result == 0x00u) {
        cpu.set_flag(Flag::zero);
    }

    if(result < 0x00u) {
        cpu.set_flag(Flag::zero);
    }

    if((acc & 0x0Fu) - (value & 0x0Fu) - carry < 0x0Fu) {
        cpu.set_flag(Flag::half_carry);
    }

    acc = result;
    return 4u;
}

uint8_t gameboy::cpu::ALU::subtract_c(const gameboy::cpu::Register8& reg) const
{
    return subtract_c(reg.get_value());
}

uint8_t gameboy::cpu::ALU::increment(uint8_t& value) const
{
    ++value;

    cpu.reset_flag(Flag::subtract);
    if(value == 0x00u) {
        cpu.set_flag(Flag::zero);
    } else {
        cpu.reset_flag(Flag::zero);
    }

    if((value & 0x0Fu) != 0x00u) {
        cpu.reset_flag(Flag::half_carry);
    } else {
        cpu.set_flag(Flag::half_carry);
    }

    return 4u;
}

uint8_t gameboy::cpu::ALU::increment(gameboy::cpu::Register8& reg) const
{
    auto value = reg.get_value();

    const auto cycles = increment(value);
    reg = value;

    return cycles;
}

uint8_t gameboy::cpu::ALU::decrement(uint8_t& value) const
{
    --value;

    cpu.set_flag(Flag::subtract);
    if(value == 0x00u) {
        cpu.set_flag(Flag::zero);
    } else {
        cpu.reset_flag(Flag::zero);
    }

    if((value & 0x0Fu) == 0x0Fu) {
        cpu.set_flag(Flag::half_carry);
    } else {
        cpu.reset_flag(Flag::half_carry);
    }

    return 4u;
}

uint8_t gameboy::cpu::ALU::decrement(gameboy::cpu::Register8& reg) const
{
    auto value = reg.get_value();

    const auto cycles = decrement(value);
    reg = value;

    return cycles;
}

uint8_t gameboy::cpu::ALU::logical_or(const uint8_t value) const
{
    auto& acc = cpu.a_f.get_high();
    acc = acc | value;

    cpu.reset_flag(Flag::all);
    if(acc == 0x00u) {
        cpu.set_flag(Flag::zero);
    }

    return 4u;
}

uint8_t gameboy::cpu::ALU::logical_or(const gameboy::cpu::Register8& reg) const
{
    return logical_or(reg.get_value());
}

uint8_t gameboy::cpu::ALU::logical_and(const uint8_t value) const
{
    auto& acc = cpu.a_f.get_high();
    acc = acc & value;

    cpu.reset_flag(Flag::all);
    cpu.set_flag(Flag::half_carry);
    if(acc == 0x00u) {
        cpu.set_flag(Flag::zero);
    }

    return 4u;
}

uint8_t gameboy::cpu::ALU::logical_and(const gameboy::cpu::Register8& reg) const
{
    return logical_and(reg.get_value());
}

uint8_t gameboy::cpu::ALU::logical_xor(const uint8_t value) const
{
    auto& acc = cpu.a_f.get_high();
    acc = acc ^ value;

    cpu.reset_flag(Flag::all);
    if(acc == 0x00u) {
        cpu.set_flag(Flag::zero);
    }

    return 4u;
}

uint8_t gameboy::cpu::ALU::logical_xor(const gameboy::cpu::Register8& reg) const
{
    return logical_xor(reg.get_value());
}

uint8_t gameboy::cpu::ALU::logical_compare(const uint8_t value) const
{
    auto& acc = cpu.a_f.get_high();

    cpu.reset_flag(Flag::all);
    cpu.set_flag(Flag::subtract);

    if(acc == value) {
        cpu.set_flag(Flag::zero);
    }

    if((acc & 0x0Fu) < (value & 0x0Fu)) {
        cpu.set_flag(Flag::half_carry);
    }

    if(acc < value) {
        cpu.set_flag(Flag::carry);
    }

    return 4u;
}

uint8_t gameboy::cpu::ALU::logical_compare(const gameboy::cpu::Register8& reg) const
{
    return logical_compare(reg.get_value());
}

uint8_t gameboy::cpu::ALU::add(gameboy::cpu::Register16& r_left, const gameboy::cpu::Register16& r_right) const
{
    cpu.reset_flag(Flag::subtract);

    if((r_left & 0x0FFFu) + (r_right & 0x0FFFu) > 0x0FFFu) {
        cpu.set_flag(Flag::half_carry);
    } else {
        cpu.reset_flag(Flag::half_carry);
    }

    if(r_left + r_right > 0xFFFFu) {
        cpu.set_flag(Flag::carry);
    } else {
        cpu.reset_flag(Flag::carry);
    }

    r_left += r_right;
    return 8u;
}

uint8_t gameboy::cpu::ALU::add_to_stack_pointer(const int8_t immediate) const
{
    cpu.reset_flag(Flag::all);

    // signed arithmetic
    if((cpu.stack_pointer & 0x0F).get_value() + (immediate & 0x0F) > 0x0Fu) {
        cpu.set_flag(Flag::half_carry);
    }

    // signed arithmetic
    if((cpu.stack_pointer & 0xFF).get_value() + (immediate & 0xFF) > 0xFFu) {
        cpu.set_flag(Flag::carry);
    }

    cpu.stack_pointer = static_cast<uint16_t>(cpu.stack_pointer.get_value() + immediate);

    return 16u;
}

uint8_t gameboy::cpu::ALU::increment(gameboy::cpu::Register16& r)
{
    r += 0x1u;
    return 8u;
}

uint8_t gameboy::cpu::ALU::decrement(gameboy::cpu::Register16& r)
{
    r -= 0x1u;
    return 8u;
}

uint8_t gameboy::cpu::ALU::complement() const
{
    cpu.a_f.get_high() = ~cpu.a_f.get_high();
    cpu.set_flag(Flag::subtract);
    cpu.set_flag(Flag::half_carry);
    return 4u;
}

uint8_t gameboy::cpu::ALU::decimal_adjust() const
{
    uint16_t acc = cpu.a_f.get_high().get_value();

    if(cpu.test_flag(Flag::subtract)) {
        if(cpu.test_flag(Flag::half_carry)) {
            acc = (acc - 0x06u) & 0xFFu;
        }

        if(cpu.test_flag(Flag::carry)) {
            acc -= 0x60u;
        }
    } else {
        if(cpu.test_flag(Flag::half_carry) || (acc & 0x0Fu) > 0x09u) {
            acc += 0x06u;
        }

        if(cpu.test_flag(Flag::carry) || acc > 0x9F) {
            acc += 0x60u;
        }
    }

    cpu.reset_flag(Flag::half_carry);
    cpu.reset_flag(Flag::zero);

    if((acc & 0x100u) == 0x100u) {
        cpu.set_flag(Flag::carry);
    }

    acc &= 0xFFu;

    if(acc == 0x00u) {
        cpu.set_flag(Flag::zero);
    }

    cpu.a_f.get_high() = static_cast<uint8_t>(acc);

    return 4u;
}

uint8_t gameboy::cpu::ALU::swap(uint8_t& value) const
{
    cpu.reset_flag(Flag::all);

    if(value == 0x00u) {
        cpu.set_flag(Flag::zero);
    } else {
        value = (value << 0x4u) | (value >> 0x4u);
    }

    return 8u;
}

uint8_t gameboy::cpu::ALU::swap(gameboy::cpu::Register8& reg) const
{
    auto value = reg.get_value();

    const auto cycles = swap(value);
    reg = value;

    return cycles;
}

uint8_t gameboy::cpu::ALU::bit_test(const uint8_t value, const uint8_t bit) const
{
    if(math::bit_test(value, bit)) {
        cpu.set_flag(Flag::zero);
    } else {
        cpu.reset_flag(Flag::zero);
    }

    cpu.set_flag(Flag::half_carry);
    cpu.reset_flag(Flag::subtract);
    return 8u;
}

uint8_t gameboy::cpu::ALU::bit_test(const gameboy::cpu::Register8& reg, const uint8_t bit) const
{
    return bit_test(reg.get_value(), bit);
}

uint8_t gameboy::cpu::ALU::bit_set(uint8_t& value, const uint8_t bit)
{
    math::bit_set(value, bit);
    return 8u;
}

uint8_t gameboy::cpu::ALU::bit_set(gameboy::cpu::Register8& reg, uint8_t bit) const
{
    auto value = reg.get_value();
    const auto cycles = bit_set(value, bit);
    reg = value;
    return cycles;
}

uint8_t gameboy::cpu::ALU::bit_reset(uint8_t& value, const uint8_t bit)
{
    math::bit_reset(value, bit);
    return 8u;
}

uint8_t gameboy::cpu::ALU::bit_reset(gameboy::cpu::Register8& reg, const uint8_t bit) const
{
    auto value = reg.get_value();
    const auto cycles = bit_reset(value, bit);
    reg = value;
    return cycles;
}

uint8_t gameboy::cpu::ALU::rotate_left_acc() const
{
    rotate_left(cpu.a_f.get_high());
    cpu.reset_flag(Flag::zero);
    return 4u;
}

uint8_t gameboy::cpu::ALU::rotate_right_acc() const
{
    rotate_right(cpu.a_f.get_high());
    cpu.reset_flag(Flag::zero);
    return 4u;
}

uint8_t gameboy::cpu::ALU::rotate_left_c_acc() const
{
    rotate_left_c(cpu.a_f.get_high());
    cpu.reset_flag(Flag::zero);
    return 4u;
}

uint8_t gameboy::cpu::ALU::rotate_right_c_acc() const
{
    rotate_right_c(cpu.a_f.get_high());
    cpu.reset_flag(Flag::zero);
    return 4u;
}

uint8_t gameboy::cpu::ALU::rotate_left(uint8_t& value) const
{
    const auto msb = value & 0x80u;
    value <<= 0x1u;

    if(cpu.test_flag(Flag::carry)) {
        bit_set(value, 0u);
    } else {
        bit_reset(value, 0u);
    }

    cpu.reset_flag(Flag::all);
    if(msb != 0x00u) {
        cpu.set_flag(Flag::carry);
    }

    if(value == 0x00u) {
        cpu.set_flag(Flag::zero);
    }

    return 8u;
}

uint8_t gameboy::cpu::ALU::rotate_left(gameboy::cpu::Register8& reg) const
{
    auto value = reg.get_value();
    const auto cycles = rotate_left(value);
    reg = value;
    return cycles;
}

uint8_t gameboy::cpu::ALU::rotate_right(uint8_t& value) const
{
    const auto lsb = value & 0x01u;
    value >>= 0x1u;

    if(cpu.test_flag(Flag::carry)) {
        bit_set(value, 7u);
    } else {
        bit_reset(value, 7u);
    }

    cpu.reset_flag(Flag::all);
    if(lsb == 0x01u) {
        cpu.set_flag(Flag::carry);
    }

    if(value == 0x00u) {
        cpu.set_flag(Flag::zero);
    }

    return 8u;
}

uint8_t gameboy::cpu::ALU::rotate_right(gameboy::cpu::Register8& reg) const
{
    auto value = reg.get_value();
    const auto cycles = rotate_right(value);
    reg = value;
    return cycles;
}

uint8_t gameboy::cpu::ALU::rotate_left_c(uint8_t& value) const
{
    const auto msb = value & 0x80u;
    value <<= 0x1u;

    cpu.reset_flag(Flag::all);
    if(msb != 0x00u) {
        cpu.set_flag(Flag::carry);
        bit_set(value, 0u);
    } else {
        bit_reset(value, 0u);
    }

    if(value == 0x00u) {
        cpu.set_flag(Flag::zero);
    }

    return 8u;
}

uint8_t gameboy::cpu::ALU::rotate_left_c(gameboy::cpu::Register8& reg) const
{
    auto value = reg.get_value();
    const auto cycles = rotate_left_c(value);
    reg = value;
    return cycles;
}

uint8_t gameboy::cpu::ALU::rotate_right_c(uint8_t& value) const
{
    const auto lsb = value & 0x01u;
    value >>= 0x1u;

    cpu.reset_flag(Flag::all);
    if(lsb != 0x00u) {
        cpu.set_flag(Flag::carry);
        bit_set(value, 7u);
    } else {
        bit_reset(value, 7u);
    }

    if(value == 0x00u) {
        cpu.set_flag(Flag::zero);
    }

    return 8u;
}

uint8_t gameboy::cpu::ALU::rotate_right_c(gameboy::cpu::Register8& reg) const
{
    auto value = reg.get_value();
    const auto cycles = rotate_right_c(value);
    reg = value;
    return cycles;
}

uint8_t gameboy::cpu::ALU::shift_left(uint8_t& value) const
{
    const auto msb = value & 0x80u;
    value <<= 0x1u;

    cpu.reset_flag(Flag::all);
    if(msb != 0x00u) {
        cpu.set_flag(Flag::carry);
    }

    if(value == 0x00u) {
        cpu.set_flag(Flag::zero);
    }

    return 8u;
}

uint8_t gameboy::cpu::ALU::shift_left(gameboy::cpu::Register8& reg) const
{
    auto value = reg.get_value();
    const auto cycles = shift_left(value);
    reg = value;
    return cycles;
}

uint8_t gameboy::cpu::ALU::shift_right(uint8_t& value, PreserveLastBit) const
{
    const auto msb = value & 0x80u;
    const auto lsb = value & 0x01u;

    value >>= 0x1u;

    if(msb != 0x00u) {
        bit_set(value, 7u);
    } else {
        bit_reset(value, 7u);
    }

    cpu.reset_flag(Flag::all);
    if(lsb == 0x01u) {
        cpu.set_flag(Flag::carry);
    }

    if(value == 0x00u) {
        cpu.set_flag(Flag::zero);
    }

    return 8u;
}

uint8_t gameboy::cpu::ALU::shift_right(gameboy::cpu::Register8& reg, const PreserveLastBit tag) const
{
    auto value = reg.get_value();
    const auto cycles = shift_right(value, tag);
    reg = value;
    return cycles;
}

uint8_t gameboy::cpu::ALU::shift_right(uint8_t& value, ResetLastBit) const
{
    const auto lsb = value & 0x01u;

    value >>= 0x1u;
    bit_reset(value, 7u);

    cpu.reset_flag(Flag::all);
    if(lsb == 0x01u) {
        cpu.set_flag(Flag::carry);
    }

    if(value == 0x00u) {
        cpu.set_flag(Flag::zero);
    }

    return 8u;
}

uint8_t gameboy::cpu::ALU::shift_right(gameboy::cpu::Register8& reg, const ResetLastBit tag) const
{
    auto value = reg.get_value();
    const auto cycles = shift_right(value, tag);
    reg = value;
    return cycles;
}
