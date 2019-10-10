#include <cpu/alu.h>
#include <cpu/cpu.h>
#include <util/math.h>

uint8_t gameboy::alu::add(const uint8_t value) const
{
    auto& acc = cpu_->a_f_.get_high();

    cpu_->reset_flag(cpu::flag::all);
    if(((acc + value) & 0xFFu) == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }

    if((acc & 0x0Fu) + (value & 0x0Fu) > 0x0Fu) {
        cpu_->set_flag(cpu::flag::half_carry);
    }

    if(acc + value > 0xFFu) {
        cpu_->set_flag(cpu::flag::carry);
    }

    acc += value;
    return 4u;
}

uint8_t gameboy::alu::add(const gameboy::register8& reg) const
{
    return add(reg.get_value());
}

uint8_t gameboy::alu::add_c(const uint8_t value) const
{
    auto& acc = cpu_->a_f_.get_high();

    const uint8_t carry = cpu_->test_flag(cpu::flag::carry) ? 0x01u : 0x00u;
    const uint16_t result = acc + value + carry;

    cpu_->reset_flag(cpu::flag::all);

    if((result & 0xFFu) == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }

    if((acc & 0x0Fu) + (value & 0x0Fu) + carry > 0x0Fu) {
        cpu_->set_flag(cpu::flag::half_carry);
    }

    if(result > 0xFFu) {
        cpu_->set_flag(cpu::flag::carry);
    }

    acc = result & 0xFFu;
    return 4u;
}

uint8_t gameboy::alu::add_c(const gameboy::register8& reg) const
{
    return add_c(reg.get_value());
}

uint8_t gameboy::alu::subtract(const uint8_t value) const
{
    auto& acc = cpu_->a_f_.get_high();

    cpu_->reset_flag(cpu::flag::all);
    cpu_->set_flag(cpu::flag::subtract);

    if(acc - value == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }

    if((acc & 0x0Fu) < (value & 0x0Fu)) {
        cpu_->set_flag(cpu::flag::half_carry);
    }

    if(acc < value) {
        cpu_->set_flag(cpu::flag::carry);
    }

    acc -= value;
    return 4u;
}

uint8_t gameboy::alu::subtract(const gameboy::register8& reg) const
{
    return subtract(reg.get_value());
}

uint8_t gameboy::alu::subtract_c(const uint8_t value) const
{
    auto& acc = cpu_->a_f_.get_high();

    const auto carry = cpu_->test_flag(cpu::flag::carry) ? 0x01u : 0x00u;
    const auto result = acc - value - carry;

    cpu_->reset_flag(cpu::flag::all);
    cpu_->set_flag(cpu::flag::subtract);

    if(result == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }

    if(result < 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }

    if((acc & 0x0Fu) - (value & 0x0Fu) - carry < 0x0Fu) {
        cpu_->set_flag(cpu::flag::half_carry);
    }

    acc = result;
    return 4u;
}

uint8_t gameboy::alu::subtract_c(const gameboy::register8& reg) const
{
    return subtract_c(reg.get_value());
}

uint8_t gameboy::alu::increment(uint8_t& value) const
{
    ++value;

    cpu_->reset_flag(cpu::flag::subtract);
    if(value == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    } else {
        cpu_->reset_flag(cpu::flag::zero);
    }

    if((value & 0x0Fu) != 0x00u) {
        cpu_->reset_flag(cpu::flag::half_carry);
    } else {
        cpu_->set_flag(cpu::flag::half_carry);
    }

    return 4u;
}

uint8_t gameboy::alu::increment(gameboy::register8& reg) const
{
    auto value = reg.get_value();

    const auto cycles = increment(value);
    reg = value;

    return cycles;
}

uint8_t gameboy::alu::decrement(uint8_t& value) const
{
    --value;

    cpu_->set_flag(cpu::flag::subtract);
    if(value == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    } else {
        cpu_->reset_flag(cpu::flag::zero);
    }

    if((value & 0x0Fu) == 0x0Fu) {
        cpu_->set_flag(cpu::flag::half_carry);
    } else {
        cpu_->reset_flag(cpu::flag::half_carry);
    }

    return 4u;
}

uint8_t gameboy::alu::decrement(gameboy::register8& reg) const
{
    auto value = reg.get_value();

    const auto cycles = decrement(value);
    reg = value;

    return cycles;
}

uint8_t gameboy::alu::logical_or(const uint8_t value) const
{
    auto& acc = cpu_->a_f_.get_high();
    acc = acc | value;

    cpu_->reset_flag(cpu::flag::all);
    if(acc == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }

    return 4u;
}

uint8_t gameboy::alu::logical_or(const gameboy::register8& reg) const
{
    return logical_or(reg.get_value());
}

uint8_t gameboy::alu::logical_and(const uint8_t value) const
{
    auto& acc = cpu_->a_f_.get_high();
    acc = acc & value;

    cpu_->reset_flag(cpu::flag::all);
    cpu_->set_flag(cpu::flag::half_carry);
    if(acc == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }

    return 4u;
}

uint8_t gameboy::alu::logical_and(const gameboy::register8& reg) const
{
    return logical_and(reg.get_value());
}

uint8_t gameboy::alu::logical_xor(const uint8_t value) const
{
    auto& acc = cpu_->a_f_.get_high();
    acc = acc ^ value;

    cpu_->reset_flag(cpu::flag::all);
    if(acc == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }

    return 4u;
}

uint8_t gameboy::alu::logical_xor(const gameboy::register8& reg) const
{
    return logical_xor(reg.get_value());
}

uint8_t gameboy::alu::logical_compare(const uint8_t value) const
{
    auto& acc = cpu_->a_f_.get_high();

    cpu_->reset_flag(cpu::flag::all);
    cpu_->set_flag(cpu::flag::subtract);

    if(acc == value) {
        cpu_->set_flag(cpu::flag::zero);
    }

    if((acc & 0x0Fu) < (value & 0x0Fu)) {
        cpu_->set_flag(cpu::flag::half_carry);
    }

    if(acc < value) {
        cpu_->set_flag(cpu::flag::carry);
    }

    return 4u;
}

uint8_t gameboy::alu::logical_compare(const gameboy::register8& reg) const
{
    return logical_compare(reg.get_value());
}

uint8_t gameboy::alu::add(gameboy::register16& r_left, const gameboy::register16& r_right) const
{
    cpu_->reset_flag(cpu::flag::subtract);

    if((r_left & 0x0FFFu) + (r_right & 0x0FFFu) > 0x0FFFu) {
        cpu_->set_flag(cpu::flag::half_carry);
    } else {
        cpu_->reset_flag(cpu::flag::half_carry);
    }

    if(r_left + r_right > 0xFFFFu) {
        cpu_->set_flag(cpu::flag::carry);
    } else {
        cpu_->reset_flag(cpu::flag::carry);
    }

    r_left += r_right;
    return 8u;
}

uint8_t gameboy::alu::add_to_stack_pointer(const int8_t immediate) const
{
    cpu_->reset_flag(cpu::flag::all);

    // signed arithmetic
    if((cpu_->stack_pointer_ & 0x0F).get_value() + (immediate & 0x0F) > 0x0Fu) {
        cpu_->set_flag(cpu::flag::half_carry);
    }

    // signed arithmetic
    if((cpu_->stack_pointer_ & 0xFF).get_value() + (immediate & 0xFF) > 0xFFu) {
        cpu_->set_flag(cpu::flag::carry);
    }

    cpu_->stack_pointer_ = static_cast<uint16_t>(cpu_->stack_pointer_.get_value() + immediate);

    return 16u;
}

uint8_t gameboy::alu::increment(gameboy::register16& r)
{
    r += 0x1u;
    return 8u;
}

uint8_t gameboy::alu::decrement(gameboy::register16& r)
{
    r -= 0x1u;
    return 8u;
}

uint8_t gameboy::alu::complement() const
{
    cpu_->a_f_.get_high() = ~cpu_->a_f_.get_high();
    cpu_->set_flag(cpu::flag::subtract);
    cpu_->set_flag(cpu::flag::half_carry);
    return 4u;
}

uint8_t gameboy::alu::decimal_adjust() const
{
    uint16_t acc = cpu_->a_f_.get_high().get_value();

    if(cpu_->test_flag(cpu::flag::subtract)) {
        if(cpu_->test_flag(cpu::flag::half_carry)) {
            acc = (acc - 0x06u) & 0xFFu;
        }

        if(cpu_->test_flag(cpu::flag::carry)) {
            acc -= 0x60u;
        }
    } else {
        if(cpu_->test_flag(cpu::flag::half_carry) || (acc & 0x0Fu) > 0x09u) {
            acc += 0x06u;
        }

        if(cpu_->test_flag(cpu::flag::carry) || acc > 0x9F) {
            acc += 0x60u;
        }
    }

    cpu_->reset_flag(cpu::flag::half_carry);
    cpu_->reset_flag(cpu::flag::zero);

    if((acc & 0x100u) == 0x100u) {
        cpu_->set_flag(cpu::flag::carry);
    }

    acc &= 0xFFu;

    if(acc == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }

    cpu_->a_f_.get_high() = static_cast<uint8_t>(acc);

    return 4u;
}

uint8_t gameboy::alu::swap(uint8_t& value) const
{
    cpu_->reset_flag(cpu::flag::all);

    if(value == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    } else {
        value = (value << 0x4u) | (value >> 0x4u);
    }

    return 8u;
}

uint8_t gameboy::alu::swap(gameboy::register8& reg) const
{
    auto value = reg.get_value();

    const auto cycles = swap(value);
    reg = value;

    return cycles;
}

uint8_t gameboy::alu::bit_test(const uint8_t value, const uint8_t bit) const
{
    if(math::bit_test(value, bit)) {
        cpu_->set_flag(cpu::flag::zero);
    } else {
        cpu_->reset_flag(cpu::flag::zero);
    }

    cpu_->set_flag(cpu::flag::half_carry);
    cpu_->reset_flag(cpu::flag::subtract);
    return 8u;
}

uint8_t gameboy::alu::bit_test(const gameboy::register8& reg, const uint8_t bit) const
{
    return bit_test(reg.get_value(), bit);
}

uint8_t gameboy::alu::bit_set(uint8_t& value, const uint8_t bit)
{
    math::bit_set(value, bit);
    return 8u;
}

uint8_t gameboy::alu::bit_set(gameboy::register8& reg, uint8_t bit) const
{
    auto value = reg.get_value();
    const auto cycles = bit_set(value, bit);
    reg = value;
    return cycles;
}

uint8_t gameboy::alu::bit_reset(uint8_t& value, const uint8_t bit)
{
    math::bit_reset(value, bit);
    return 8u;
}

uint8_t gameboy::alu::bit_reset(gameboy::register8& reg, const uint8_t bit) const
{
    auto value = reg.get_value();
    const auto cycles = bit_reset(value, bit);
    reg = value;
    return cycles;
}

uint8_t gameboy::alu::rotate_left_acc() const
{
    rotate_left(cpu_->a_f_.get_high());
    cpu_->reset_flag(cpu::flag::zero);
    return 4u;
}

uint8_t gameboy::alu::rotate_right_acc() const
{
    rotate_right(cpu_->a_f_.get_high());
    cpu_->reset_flag(cpu::flag::zero);
    return 4u;
}

uint8_t gameboy::alu::rotate_left_c_acc() const
{
    rotate_left_c(cpu_->a_f_.get_high());
    cpu_->reset_flag(cpu::flag::zero);
    return 4u;
}

uint8_t gameboy::alu::rotate_right_c_acc() const
{
    rotate_right_c(cpu_->a_f_.get_high());
    cpu_->reset_flag(cpu::flag::zero);
    return 4u;
}

uint8_t gameboy::alu::rotate_left(uint8_t& value) const
{
    const auto msb = value & 0x80u;
    value <<= 0x1u;

    if(cpu_->test_flag(cpu::flag::carry)) {
        bit_set(value, 0u);
    } else {
        bit_reset(value, 0u);
    }

    cpu_->reset_flag(cpu::flag::all);
    if(msb != 0x00u) {
        cpu_->set_flag(cpu::flag::carry);
    }

    if(value == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }

    return 8u;
}

uint8_t gameboy::alu::rotate_left(gameboy::register8& reg) const
{
    auto value = reg.get_value();
    const auto cycles = rotate_left(value);
    reg = value;
    return cycles;
}

uint8_t gameboy::alu::rotate_right(uint8_t& value) const
{
    const auto lsb = value & 0x01u;
    value >>= 0x1u;

    if(cpu_->test_flag(cpu::flag::carry)) {
        bit_set(value, 7u);
    } else {
        bit_reset(value, 7u);
    }

    cpu_->reset_flag(cpu::flag::all);
    if(lsb == 0x01u) {
        cpu_->set_flag(cpu::flag::carry);
    }

    if(value == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }

    return 8u;
}

uint8_t gameboy::alu::rotate_right(gameboy::register8& reg) const
{
    auto value = reg.get_value();
    const auto cycles = rotate_right(value);
    reg = value;
    return cycles;
}

uint8_t gameboy::alu::rotate_left_c(uint8_t& value) const
{
    const auto msb = value & 0x80u;
    value <<= 0x1u;

    cpu_->reset_flag(cpu::flag::all);
    if(msb != 0x00u) {
        cpu_->set_flag(cpu::flag::carry);
        bit_set(value, 0u);
    } else {
        bit_reset(value, 0u);
    }

    if(value == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }

    return 8u;
}

uint8_t gameboy::alu::rotate_left_c(gameboy::register8& reg) const
{
    auto value = reg.get_value();
    const auto cycles = rotate_left_c(value);
    reg = value;
    return cycles;
}

uint8_t gameboy::alu::rotate_right_c(uint8_t& value) const
{
    const auto lsb = value & 0x01u;
    value >>= 0x1u;

    cpu_->reset_flag(cpu::flag::all);
    if(lsb != 0x00u) {
        cpu_->set_flag(cpu::flag::carry);
        bit_set(value, 7u);
    } else {
        bit_reset(value, 7u);
    }

    if(value == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }

    return 8u;
}

uint8_t gameboy::alu::rotate_right_c(gameboy::register8& reg) const
{
    auto value = reg.get_value();
    const auto cycles = rotate_right_c(value);
    reg = value;
    return cycles;
}

uint8_t gameboy::alu::shift_left(uint8_t& value) const
{
    const auto msb = value & 0x80u;
    value <<= 0x1u;

    cpu_->reset_flag(cpu::flag::all);
    if(msb != 0x00u) {
        cpu_->set_flag(cpu::flag::carry);
    }

    if(value == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }

    return 8u;
}

uint8_t gameboy::alu::shift_left(gameboy::register8& reg) const
{
    auto value = reg.get_value();
    const auto cycles = shift_left(value);
    reg = value;
    return cycles;
}

uint8_t gameboy::alu::shift_right(uint8_t& value, preserve_last_bit_t) const
{
    const auto msb = value & 0x80u;
    const auto lsb = value & 0x01u;

    value >>= 0x1u;

    if(msb != 0x00u) {
        bit_set(value, 7u);
    } else {
        bit_reset(value, 7u);
    }

    cpu_->reset_flag(cpu::flag::all);
    if(lsb == 0x01u) {
        cpu_->set_flag(cpu::flag::carry);
    }

    if(value == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }

    return 8u;
}

uint8_t gameboy::alu::shift_right(gameboy::register8& reg, const preserve_last_bit_t tag) const
{
    auto value = reg.get_value();
    const auto cycles = shift_right(value, tag);
    reg = value;
    return cycles;
}

uint8_t gameboy::alu::shift_right(uint8_t& value, reset_last_bit_t) const
{
    const auto lsb = value & 0x01u;

    value >>= 0x1u;
    bit_reset(value, 7u);

    cpu_->reset_flag(cpu::flag::all);
    if(lsb == 0x01u) {
        cpu_->set_flag(cpu::flag::carry);
    }

    if(value == 0x00u) {
        cpu_->set_flag(cpu::flag::zero);
    }

    return 8u;
}

uint8_t gameboy::alu::shift_right(gameboy::register8& reg, const reset_last_bit_t tag) const
{
    auto value = reg.get_value();
    const auto cycles = shift_right(value, tag);
    reg = value;
    return cycles;
}
