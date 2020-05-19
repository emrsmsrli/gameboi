#include "gameboy/cpu/cpu.h"

#include <spdlog/spdlog.h>

#include "gameboy/bus.h"
#include "gameboy/cartridge.h"
#include "gameboy/cpu/instruction_info.h"
#include "gameboy/memory/mmu.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

BITMASK(interrupt)

constexpr address16 ie_addr{0xFFFFu};
constexpr address16 if_addr{0xFF0Fu};
constexpr address16 key_1_addr{0xFF4Du};

cpu::cpu(const observer<bus> bus) noexcept
    : bus_{bus},
      alu_{make_observer(this)},
      a_f_(bus_->get_cartridge()->cgb_enabled() ? 0x1180u : 0x01B0u),
      b_c_(bus_->get_cartridge()->cgb_enabled() ? 0x0000u : 0x0013u),
      d_e_(bus_->get_cartridge()->cgb_enabled() ? 0xFF56u : 0x00D8u),
      h_l_(bus_->get_cartridge()->cgb_enabled() ? 0x000Du : 0x014Du),
      stack_pointer_{0xFFFEu},
      program_counter_{0x0100u},
      total_cycles_{0u},
      interrupt_flags_{bus_->get_cartridge()->cgb_enabled() ? interrupt::lcd_vblank : interrupt::none},
      interrupt_enable_{interrupt::none},
      interrupt_master_enable_{false},
      pending_disable_interrupts_counter_{-1},
      pending_enable_interrupts_counter_{-1},
      is_stopped_{false},
      is_halted_{false},
      wait_before_unhalt_cycles_{0},
      extra_cycles_{0u}
{
    auto mmu = bus->get_mmu();

    mmu->add_memory_delegate(ie_addr, {
        {connect_arg<&cpu::on_ie_read>, this},
        {connect_arg<&cpu::on_ie_write>, this},
    });

    mmu->add_memory_delegate(if_addr, {
        {connect_arg<&cpu::on_if_read>, this},
        {connect_arg<&cpu::on_if_write>, this},
    });

    mmu->add_memory_delegate(key_1_addr, {
        {connect_arg<&cpu::on_key_1_read>, this},
        {connect_arg<&cpu::on_key_1_write>, this},
    });
}

void cpu::on_ie_write(const address16&, const uint8_t data) noexcept
{
    interrupt_enable_ = static_cast<interrupt>(data);
}

uint8_t cpu::on_ie_read(const address16&) const noexcept
{
    return static_cast<uint8_t>(interrupt_enable_);
}

void cpu::on_if_write(const address16&, uint8_t data) noexcept
{
    interrupt_flags_ = static_cast<interrupt>(data);
}

uint8_t cpu::on_if_read(const address16&) const noexcept
{
    return static_cast<uint8_t>(interrupt_flags_) | 0xE0u;
}

void cpu::on_key_1_write(const address16&, const uint8_t data) noexcept
{
    if(!bus_->get_cartridge()->cgb_enabled()) {
        return;
    }

    key_1_ = (data & 0x01u) | 0x7Eu;
}

uint8_t cpu::on_key_1_read(const address16&) const noexcept
{
    return key_1_.value();
}

uint8_t cpu::tick()
{
#if WITH_DEBUGGER
    prev_program_counter_ = program_counter_;
#endif //WITH_DEBUGGER

    const auto execute_next_op = [&]() -> uint8_t {
        const auto opcode = read_immediate(imm8);
        if(opcode != 0xCB) {
            return decode(opcode, standard_instruction_set);
        }

        return decode(read_immediate(imm8), extended_instruction_set);
    };

    auto cycle_count = !is_halted_
        ? execute_next_op()
        : static_cast<uint8_t>(4u);

    if(wait_before_unhalt_cycles_ > 0) {
        wait_before_unhalt_cycles_ -= cycle_count;

        if(wait_before_unhalt_cycles_ <= 0) {
            wait_before_unhalt_cycles_ = 0;
            is_halted_ = false;
        }
    }

    if(!is_halted_) {
        if(pending_enable_interrupts_counter_ != -1) {
            if(pending_enable_interrupts_counter_ == 0) {
               interrupt_master_enable_ = true;
            }

            --pending_enable_interrupts_counter_;
        }

        if(pending_disable_interrupts_counter_ != -1) {
            if(pending_disable_interrupts_counter_ == 0) {
                interrupt_master_enable_ = false;
            }

            --pending_disable_interrupts_counter_;
        }
    }

    cycle_count += extra_cycles_;
    extra_cycles_ = 0;

    if(is_in_double_speed()) {
        cycle_count /= 2;
    }

    total_cycles_ += cycle_count;
    return cycle_count;
}

void cpu::process_interrupts() noexcept
{
    const auto pending = interrupt_enable_ & interrupt_flags_;
    const auto int_requested = [&](const interrupt i) {
        return (pending & i) != interrupt::none;
    };

    static constexpr std::array interrupts{
        interrupt::joypad,
        interrupt::serial,
        interrupt::timer,
        interrupt::lcd_stat,
        interrupt::lcd_vblank
    };

    if(const auto it = std::find_if(begin(interrupts), end(interrupts), int_requested); it != end(interrupts)) {
        const auto interrupt_request = *it;

        if(is_stopped_ && interrupt_request == interrupt::joypad) {
            is_stopped_ = false;
            is_halted_ = false;
        }

        if(is_halted_ && wait_before_unhalt_cycles_ == 0) {
            wait_before_unhalt_cycles_ = 12;
        }

        if(interrupt_master_enable_) {
            interrupt_master_enable_ = false;
            interrupt_flags_ &= ~interrupt_request;
            rst(make_address(interrupt_request));
            extra_cycles_ = 20;
        }
    }
}

void cpu::request_interrupt(const interrupt request) noexcept
{
    interrupt_flags_ |= request;
}

bool cpu::is_in_double_speed() const noexcept
{
    return bit::test(key_1_, 7u) && !bit::test(key_1_, 0u);
}

void cpu::set_flag(const flag flag) noexcept
{
    a_f_.low() |= static_cast<uint8_t>(flag);
}

void cpu::reset_flag(const flag flag) noexcept
{
    a_f_.low() &= ~static_cast<uint8_t>(flag);
}

void cpu::flip_flag(const flag flag) noexcept
{
    a_f_.low() ^= static_cast<uint8_t>(flag);
}

bool cpu::test_flag(const flag flag) noexcept
{
    const auto f = static_cast<uint8_t>(flag);
    return mask::test(a_f_.low(), f);
}

uint8_t cpu::decode(const uint8_t inst, standard_instruction_set_t)
{
    const auto info = instruction::standard_instruction_set[inst];

    const auto data = [&]() -> uint16_t {
        switch(info.length) {
            case 3u: return read_immediate(imm16);
            case 2u: return read_immediate(imm8);
            default: return 0u;
        }
    }();

    const auto false_branch = [&]() {
#if WITH_DEBUGGER
        if(on_instruction_executed_) {
            on_instruction_executed_(make_address(prev_program_counter_), info, data);
        }
#endif //WITH_DEBUGGER

        return instruction::get_false_branch_cycle_count(inst);
    };

    switch(inst) {
        case 0x00: {
            nop();
            break;
        }
        case 0x01: {
            load(b_c_, data);
            break;
        }
        case 0x02: {
            store(make_address(b_c_), a_f_.high());
            break;
        }
        case 0x03: {
            alu::increment(b_c_);
            break;
        }
        case 0x04: {
            alu_.increment(b_c_.high());
            break;
        }
        case 0x05: {
            alu_.decrement(b_c_.high());
            break;
        }
        case 0x06: {
            load(b_c_.high(), static_cast<uint8_t>(data));
            break;
        }
        case 0x07: {
            alu_.rotate_left_c_acc();
            break;
        }
        case 0x08: {
            store(make_address(data), stack_pointer_);
            break;
        }
        case 0x09: {
            alu_.add(h_l_, b_c_);
            break;
        }
        case 0x0A: {
            load(a_f_.high(), read_data(make_address(b_c_)));
            break;
        }
        case 0x0B: {
            alu::decrement(b_c_);
            break;
        }
        case 0x0C: {
            alu_.increment(b_c_.low());
            break;
        }
        case 0x0D: {
            alu_.decrement(b_c_.low());
            break;
        }
        case 0x0E: {
            load(b_c_.low(), static_cast<uint8_t>(data));
            break;
        }
        case 0x0F: {
            alu_.rotate_right_c_acc();
            break;
        }
        case 0x10: {
            stop();
            break;
        }
        case 0x11: {
            load(d_e_, data);
            break;
        }
        case 0x12: {
            store(make_address(d_e_), a_f_.high());
            break;
        }
        case 0x13: {
            alu::increment(d_e_);
            break;
        }
        case 0x14: {
            alu_.increment(d_e_.high());
            break;
        }
        case 0x15: {
            alu_.decrement(d_e_.high());
            break;
        }
        case 0x16: {
            load(d_e_.high(), static_cast<uint8_t>(data));
            break;
        }
        case 0x17: {
            alu_.rotate_left_acc();
            break;
        }
        case 0x18: {
            jump_relative(make_address(static_cast<uint8_t>(data)));
            break;
        }
        case 0x19: {
            alu_.add(h_l_, d_e_);
            break;
        }
        case 0x1A: {
            load(a_f_.high(), read_data(make_address(d_e_)));
            break;
        }
        case 0x1B: {
            alu::decrement(d_e_);
            break;
        }
        case 0x1C: {
            alu_.increment(d_e_.low());
            break;
        }
        case 0x1D: {
            alu_.decrement(d_e_.low());
            break;
        }
        case 0x1E: {
            load(d_e_.low(), static_cast<uint8_t>(data));
            break;
        }
        case 0x1F: {
            alu_.rotate_right_acc();
            break;
        }
        case 0x20: {
            if(!test_flag(flag::zero)) {
                jump_relative(make_address(static_cast<uint8_t>(data)));
            } else {
                return false_branch();
            }
            break;
        }
        case 0x21: {
            load(h_l_, data);
            break;
        }
        case 0x22: {
            store(make_address(h_l_++), a_f_.high());
            break;
        }
        case 0x23: {
            alu::increment(h_l_);
            break;
        }
        case 0x24: {
            alu_.increment(h_l_.high());
            break;
        }
        case 0x25: {
            alu_.decrement(h_l_.high());
            break;
        }
        case 0x26: {
            load(h_l_.high(), static_cast<uint8_t>(data));
            break;
        }
        case 0x27: {
            alu_.decimal_adjust();
            break;
        }
        case 0x28: {
            if(test_flag(flag::zero)) {
                jump_relative(make_address(static_cast<uint8_t>(data)));
            } else {
                return false_branch();
            }
            break;
        }
        case 0x29: {
            alu_.add(h_l_, h_l_);
            break;
        }
        case 0x2A: {
            load(a_f_.high(), read_data(make_address(h_l_++)));
            break;
        }
        case 0x2B: {
            alu::decrement(h_l_);
            break;
        }
        case 0x2C: {
            alu_.increment(h_l_.low());
            break;
        }
        case 0x2D: {
            alu_.decrement(h_l_.low());
            break;
        }
        case 0x2E: {
            load(h_l_.low(), static_cast<uint8_t>(data));
            break;
        }
        case 0x2F: {
            alu_.complement();
            break;
        }
        case 0x30: {
            if(!test_flag(flag::carry)) {
                jump_relative(make_address(static_cast<uint8_t>(data)));
            } else {
                return false_branch();
            }
            break;
        }
        case 0x31: {
            load(stack_pointer_, data);
            break;
        }
        case 0x32: {
            store(make_address(h_l_--), a_f_.high());
            break;
        }
        case 0x33: {
            alu::increment(stack_pointer_);
            break;
        }
        case 0x34: {
            const auto address = make_address(h_l_);
            auto mem_data = read_data(address);
            alu_.increment(mem_data);
            write_data(address, mem_data);
            break;
        }
        case 0x35: {
            const auto address = make_address(h_l_);
            auto mem_data = read_data(address);
            alu_.decrement(mem_data);
            write_data(address, mem_data);
            break;
        }
        case 0x36: {
            store(make_address(h_l_), static_cast<uint8_t>(data));
            break;
        }
        case 0x37: { /* SCF */
            reset_flag(flag::negative | flag::half_carry);
            set_flag(flag::carry);
            break;
        }
        case 0x38: {
            if(test_flag(flag::carry)) {
                jump_relative(make_address(static_cast<uint8_t>(data)));
            } else {
                return false_branch();
            }
            break;
        }
        case 0x39: {
            alu_.add(h_l_, stack_pointer_);
            break;
        }
        case 0x3A: {
            load(a_f_.high(), read_data(make_address(h_l_--)));
            break;
        }
        case 0x3B: {
            alu::decrement(stack_pointer_);
            break;
        }
        case 0x3C: {
            alu_.increment(a_f_.high());
            break;
        }
        case 0x3D: {
            alu_.decrement(a_f_.high());
            break;
        }
        case 0x3E: {
            load(a_f_.high(), static_cast<uint8_t>(data));
            break;
        }
        case 0x3F: {
            /* CCF */
            reset_flag(flag::negative | flag::half_carry);
            flip_flag(flag::carry);
            break;
        }
        case 0x40: {
            /* LD B,B */
            nop();
            break;
        }
        case 0x41: {
            load(b_c_.high(), b_c_.low());
            break;
        }
        case 0x42: {
            load(b_c_.high(), d_e_.high());
            break;
        }
        case 0x43: {
            load(b_c_.high(), d_e_.low());
            break;
        }
        case 0x44: {
            load(b_c_.high(), h_l_.high());
            break;
        }
        case 0x45: {
            load(b_c_.high(), h_l_.low());
            break;
        }
        case 0x46: {
            load(b_c_.high(), read_data(make_address(h_l_)));
            break;
        }
        case 0x47: {
            load(b_c_.high(), a_f_.high());
            break;
        }
        case 0x48: {
            load(b_c_.low(), b_c_.high());
            break;
        }
        case 0x49: {
            /* LD C,C */
            nop();
            break;
        }
        case 0x4A: {
            load(b_c_.low(), d_e_.high());
            break;
        }
        case 0x4B: {
            load(b_c_.low(), d_e_.low());
            break;
        }
        case 0x4C: {
            load(b_c_.low(), h_l_.high());
            break;
        }
        case 0x4D: {
            load(b_c_.low(), h_l_.low());
            break;
        }
        case 0x4E: {
            load(b_c_.low(), read_data(make_address(h_l_)));
            break;
        }
        case 0x4F: {
            load(b_c_.low(), a_f_.high());
            break;
        }
        case 0x50: {
            load(d_e_.high(), b_c_.high());
            break;
        }
        case 0x51: {
            load(d_e_.high(), b_c_.low());
            break;
        }
        case 0x52: {
            /* LD D,D */
            nop();
            break;
        }
        case 0x53: {
            load(d_e_.high(), d_e_.low());
            break;
        }
        case 0x54: {
            load(d_e_.high(), h_l_.high());
            break;
        }
        case 0x55: {
            load(d_e_.high(), h_l_.low());
            break;
        }
        case 0x56: {
            load(d_e_.high(), read_data(make_address(h_l_)));
            break;
        }
        case 0x57: {
            load(d_e_.high(), a_f_.high());
            break;
        }
        case 0x58: {
            load(d_e_.low(), b_c_.high());
            break;
        }
        case 0x59: {
            load(d_e_.low(), b_c_.low());
            break;
        }
        case 0x5A: {
            load(d_e_.low(), d_e_.high());
            break;
        }
        case 0x5B: {
            /* LD E,E */
            nop();
            break;
        }
        case 0x5C: {
            load(d_e_.low(), h_l_.high());
            break;
        }
        case 0x5D: {
            load(d_e_.low(), h_l_.low());
            break;
        }
        case 0x5E: {
            load(d_e_.low(), read_data(make_address(h_l_)));
            break;
        }
        case 0x5F: {
            load(d_e_.low(), a_f_.high());
            break;
        }
        case 0x60: {
            load(h_l_.high(), b_c_.high());
            break;
        }
        case 0x61: {
            load(h_l_.high(), b_c_.low());
            break;
        }
        case 0x62: {
            load(h_l_.high(), d_e_.high());
            break;
        }
        case 0x63: {
            load(h_l_.high(), d_e_.low());
            break;
        }
        case 0x64: {
            /* LD H, H */
            nop();
            break;
        }
        case 0x65: {
            load(h_l_.high(), h_l_.low());
            break;
        }
        case 0x66: {
            load(h_l_.high(), read_data(make_address(h_l_)));
            break;
        }
        case 0x67: {
            load(h_l_.high(), a_f_.high());
            break;
        }
        case 0x68: {
            load(h_l_.low(), b_c_.high());
            break;
        }
        case 0x69: {
            load(h_l_.low(), b_c_.low());
            break;
        }
        case 0x6A: {
            load(h_l_.low(), d_e_.high());
            break;
        }
        case 0x6B: {
            load(h_l_.low(), d_e_.low());
            break;
        }
        case 0x6C: {
            load(h_l_.low(), h_l_.high());
            break;
        }
        case 0x6D: {
            /* LD L, L */
            nop();
            break;
        }
        case 0x6E: {
            load(h_l_.low(), read_data(make_address(h_l_)));
            break;
        }
        case 0x6F: {
            load(h_l_.low(), a_f_.high());
            break;
        }
        case 0x70: {
            store(make_address(h_l_), b_c_.high());
            break;
        }
        case 0x71: {
            store(make_address(h_l_), b_c_.low());
            break;
        }
        case 0x72: {
            store(make_address(h_l_), d_e_.high());
            break;
        }
        case 0x73: {
            store(make_address(h_l_), d_e_.low());
            break;
        }
        case 0x74: {
            store(make_address(h_l_), h_l_.high());
            break;
        }
        case 0x75: {
            store(make_address(h_l_), h_l_.low());
            break;
        }
        case 0x76: {
            halt();
            break;
        }
        case 0x77: {
            store(make_address(h_l_), a_f_.high());
            break;
        }
        case 0x78: {
            load(a_f_.high(), b_c_.high());
            break;
        }
        case 0x79: {
            load(a_f_.high(), b_c_.low());
            break;
        }
        case 0x7A: {
            load(a_f_.high(), d_e_.high());
            break;
        }
        case 0x7B: {
            load(a_f_.high(), d_e_.low());
            break;
        }
        case 0x7C: {
            load(a_f_.high(), h_l_.high());
            break;
        }
        case 0x7D: {
            load(a_f_.high(), h_l_.low());
            break;
        }
        case 0x7E: {
            load(a_f_.high(), read_data(make_address(h_l_)));
            break;
        }
        case 0x7F: {
            /* LD A,A */
            nop();
            break;
        }
        case 0x80: {
            alu_.add(b_c_.high());
            break;
        }
        case 0x81: {
            alu_.add(b_c_.low());
            break;
        }
        case 0x82: {
            alu_.add(d_e_.high());
            break;
        }
        case 0x83: {
            alu_.add(d_e_.low());
            break;
        }
        case 0x84: {
            alu_.add(h_l_.high());
            break;
        }
        case 0x85: {
            alu_.add(h_l_.low());
            break;
        }
        case 0x86: {
            alu_.add(read_data(make_address(h_l_)));
            break;
        }
        case 0x87: {
            alu_.add(a_f_.high());
            break;
        }
        case 0x88: {
            alu_.add_c(b_c_.high());
            break;
        }
        case 0x89: {
            alu_.add_c(b_c_.low());
            break;
        }
        case 0x8A: {
            alu_.add_c(d_e_.high());
            break;
        }
        case 0x8B: {
            alu_.add_c(d_e_.low());
            break;
        }
        case 0x8C: {
            alu_.add_c(h_l_.high());
            break;
        }
        case 0x8D: {
            alu_.add_c(h_l_.low());
            break;
        }
        case 0x8E: {
            alu_.add_c(read_data(make_address(h_l_)));
            break;
        }
        case 0x8F: {
            alu_.add_c(a_f_.high());
            break;
        }
        case 0x90: {
            alu_.subtract(b_c_.high());
            break;
        }
        case 0x91: {
            alu_.subtract(b_c_.low());
            break;
        }
        case 0x92: {
            alu_.subtract(d_e_.high());
            break;
        }
        case 0x93: {
            alu_.subtract(d_e_.low());
            break;
        }
        case 0x94: {
            alu_.subtract(h_l_.high());
            break;
        }
        case 0x95: {
            alu_.subtract(h_l_.low());
            break;
        }
        case 0x96: {
            alu_.subtract(read_data(make_address(h_l_)));
            break;
        }
        case 0x97: {
            alu_.subtract(a_f_.high());
            break;
        }
        case 0x98: {
            alu_.subtract_c(b_c_.high());
            break;
        }
        case 0x99: {
            alu_.subtract_c(b_c_.low());
            break;
        }
        case 0x9A: {
            alu_.subtract_c(d_e_.high());
            break;
        }
        case 0x9B: {
            alu_.subtract_c(d_e_.low());
            break;
        }
        case 0x9C: {
            alu_.subtract_c(h_l_.high());
            break;
        }
        case 0x9D: {
            alu_.subtract_c(h_l_.low());
            break;
        }
        case 0x9E: {
            alu_.subtract_c(read_data(make_address(h_l_)));
            break;
        }
        case 0x9F: {
            alu_.subtract_c(a_f_.high());
            break;
        }
        case 0xA0: {
            alu_.logical_and(b_c_.high());
            break;
        }
        case 0xA1: {
            alu_.logical_and(b_c_.low());
            break;
        }
        case 0xA2: {
            alu_.logical_and(d_e_.high());
            break;
        }
        case 0xA3: {
            alu_.logical_and(d_e_.low());
            break;
        }
        case 0xA4: {
            alu_.logical_and(h_l_.high());
            break;
        }
        case 0xA5: {
            alu_.logical_and(h_l_.low());
            break;
        }
        case 0xA6: {
            alu_.logical_and(read_data(make_address(h_l_)));
            break;
        }
        case 0xA7: {
            alu_.logical_and(a_f_.high());
            break;
        }
        case 0xA8: {
            alu_.logical_xor(b_c_.high());
            break;
        }
        case 0xA9: {
            alu_.logical_xor(b_c_.low());
            break;
        }
        case 0xAA: {
            alu_.logical_xor(d_e_.high());
            break;
        }
        case 0xAB: {
            alu_.logical_xor(d_e_.low());
            break;
        }
        case 0xAC: {
            alu_.logical_xor(h_l_.high());
            break;
        }
        case 0xAD: {
            alu_.logical_xor(h_l_.low());
            break;
        }
        case 0xAE: {
            alu_.logical_xor(read_data(make_address(h_l_)));
            break;
        }
        case 0xAF: {
            alu_.logical_xor(a_f_.high());
            break;
        }
        case 0xB0: {
            alu_.logical_or(b_c_.high());
            break;
        }
        case 0xB1: {
            alu_.logical_or(b_c_.low());
            break;
        }
        case 0xB2: {
            alu_.logical_or(d_e_.high());
            break;
        }
        case 0xB3: {
            alu_.logical_or(d_e_.low());
            break;
        }
        case 0xB4: {
            alu_.logical_or(h_l_.high());
            break;
        }
        case 0xB5: {
            alu_.logical_or(h_l_.low());
            break;
        }
        case 0xB6: {
            alu_.logical_or(read_data(make_address(h_l_)));
            break;
        }
        case 0xB7: {
            alu_.logical_or(a_f_.high());
            break;
        }
        case 0xB8: {
            alu_.logical_compare(b_c_.high());
            break;
        }
        case 0xB9: {
            alu_.logical_compare(b_c_.low());
            break;
        }
        case 0xBA: {
            alu_.logical_compare(d_e_.high());
            break;
        }
        case 0xBB: {
            alu_.logical_compare(d_e_.low());
            break;
        }
        case 0xBC: {
            alu_.logical_compare(h_l_.high());
            break;
        }
        case 0xBD: {
            alu_.logical_compare(h_l_.low());
            break;
        }
        case 0xBE: {
            alu_.logical_compare(read_data(make_address(h_l_)));
            break;
        }
        case 0xBF: {
            alu_.logical_compare(a_f_.high());
            break;
        }
        case 0xC0: {
            if(!test_flag(flag::zero)) {
                ret();
            } else {
                return false_branch();
            }
            break;
        }
        case 0xC1: {
            pop(b_c_);
            break;
        }
        case 0xC2: {
            if(!test_flag(flag::zero)) {
                jump(make_address(data));
            } else {
                return false_branch();
            }
            break;
        }
        case 0xC3: {
            jump(make_address(data));
            break;
        }
        case 0xC4: {
            if(!test_flag(flag::zero)) {
                call(make_address(data));
            } else {
                return false_branch();
            }
            break;
        }
        case 0xC5: {
            push(b_c_);
            break;
        }
        case 0xC6: {
            alu_.add(static_cast<uint8_t>(data));
            break;
        }
        case 0xC7: {
            rst(address8(0x00));
            break;
        }
        case 0xC8: {
            if(test_flag(flag::zero)) {
                ret();
            } else {
                return false_branch();
            }
            break;
        }
        case 0xC9: {
            ret();
            break;
        }
        case 0xCA: {
            if(test_flag(flag::zero)) {
                jump(make_address(data));
            } else {
                return false_branch();
            }
            break;
        }
        case 0xCC: {
            if(test_flag(flag::zero)) {
                call(make_address(data));
            } else {
                return false_branch();
            }
            break;
        }
        case 0xCD: {
            call(make_address(data));
            break;
        }
        case 0xCE: {
            alu_.add_c(static_cast<uint8_t>(data));
            break;
        }
        case 0xCF: {
            rst(address8(0x08));
            break;
        }
        case 0xD0: {
            if(!test_flag(flag::carry)) {
                ret();
            } else {
                return false_branch();
            }
            break;
        }
        case 0xD1: {
            pop(d_e_);
            break;
        }
        case 0xD2: {
            if(!test_flag(flag::carry)) {
                jump(make_address(data));
            } else {
                return false_branch();
            }
            break;
        }
        case 0xD4: {
            if(!test_flag(flag::carry)) {
                call(make_address(data));
            } else {
                return false_branch();
            }
            break;
        }
        case 0xD5: {
            push(d_e_);
            break;
        }
        case 0xD6: {
            alu_.subtract(static_cast<uint8_t>(data));
            break;
        }
        case 0xD7: {
            rst(address8(0x10));
            break;
        }
        case 0xD8: {
            if(test_flag(flag::carry)) {
                ret();
            } else {
                return false_branch();
            }
            break;
        }
        case 0xD9: {
            reti();
            break;
        }
        case 0xDA: {
            if(test_flag(flag::carry)) {
                jump(make_address(data));
            } else {
                return false_branch();
            }
            break;
        }
        case 0xDC: {
            if(test_flag(flag::carry)) {
                call(make_address(data));
            } else {
                return false_branch();
            }
            break;
        }
        case 0xDE: {
            alu_.subtract_c(static_cast<uint8_t>(data));
            break;
        }
        case 0xDF: {
            rst(address8(0x18));
            break;
        }
        case 0xE0: {
            const uint16_t address = 0xFF00 + static_cast<uint8_t>(data);
            store(make_address(address), a_f_.high());
            break;
        }
        case 0xE1: {
            pop(h_l_);
            break;
        }
        case 0xE2: {
            const auto address = make_address(b_c_.low() + 0xFF00);
            store(address, a_f_.high());
            break;
        }
        case 0xE5: {
            push(h_l_);
            break;
        }
        case 0xE6: {
            alu_.logical_and(static_cast<uint8_t>(data));
            break;
        }
        case 0xE7: {
            rst(address8(0x20));
            break;
        }
        case 0xE8: {
            alu_.add_to_stack_pointer(static_cast<int8_t>(data));
            break;
        }
        case 0xE9: {
            jump(h_l_);
            break;
        }
        case 0xEA: {
            store(make_address(data), a_f_.high());
            break;
        }
        case 0xEE: {
            alu_.logical_xor(static_cast<uint8_t>(data));
            break;
        }
        case 0xEF: {
            rst(address8(0x28));
            break;
        }
        case 0xF0: {
            const uint16_t address = 0xFF00 + static_cast<uint8_t>(data);
            load(a_f_.high(), read_data(make_address(address)));
            break;
        }
        case 0xF1: {
            pop(a_f_);
            a_f_.low() &= 0xF0;
            break;
        }
        case 0xF2: {
            load(a_f_.high(), read_data(make_address(b_c_.low() + 0xFF00)));
            break;
        }
        case 0xF3: {
            pending_enable_interrupts_counter_ = -1;
            if(pending_disable_interrupts_counter_ == -1) {
                pending_disable_interrupts_counter_ = 1;
            }
            break;
        }
        case 0xF5: {
            push(a_f_);
            break;
        }
        case 0xF6: {
            alu_.logical_or(static_cast<uint8_t>(data));
            break;
        }
        case 0xF7: {
            rst(address8(0x30));
            break;
        }
        case 0xF8: {
            load_hlsp(static_cast<int8_t>(data));
            break;
        }
        case 0xF9: {
            load(stack_pointer_, h_l_);
            break;
        }
        case 0xFA: {
            load(a_f_.high(), read_data(make_address(data)));
            break;
        }
        case 0xFB: {
            pending_disable_interrupts_counter_ = -1;
            if(pending_enable_interrupts_counter_ == -1) {
                pending_enable_interrupts_counter_ = 1;
            }
            break;
        }
        case 0xFE: {
            alu_.logical_compare(static_cast<uint8_t>(data));
            break;
        }
        case 0xFF: {
            rst(address8(0x38));
            break;
        }
        default: {
            spdlog::critical("unknown instruction: {:#x}, address: {:#x}", inst, program_counter_.value() - info.length);
        }
    }

#if WITH_DEBUGGER
    if(on_instruction_executed_) {
        on_instruction_executed_(make_address(prev_program_counter_), info, data);
    }
#endif //WITH_DEBUGGER

    return info.cycle_count;
}

uint8_t cpu::decode(const uint8_t inst, extended_instruction_set_t)
{
    switch(inst) {
        case 0x00: {
            alu_.rotate_left_c(b_c_.high());
            break;
        }
        case 0x01: {
            alu_.rotate_left_c(b_c_.low());
            break;
        }
        case 0x02: {
            alu_.rotate_left_c(d_e_.high());
            break;
        }
        case 0x03: {
            alu_.rotate_left_c(d_e_.low());
            break;
        }
        case 0x04: {
            alu_.rotate_left_c(h_l_.high());
            break;
        }
        case 0x05: {
            alu_.rotate_left_c(h_l_.low());
            break;
        }
        case 0x06: {
            const auto address = make_address(h_l_);
            auto data = read_data(address);
            alu_.rotate_left_c(data);
            write_data(address, data);
            break;
        }
        case 0x07: {
            alu_.rotate_left_c(a_f_.high());
            break;
        }
        case 0x08: {
            alu_.rotate_right_c(b_c_.high());
            break;
        }
        case 0x09: {
            alu_.rotate_right_c(b_c_.low());
            break;
        }
        case 0x0A: {
            alu_.rotate_right_c(d_e_.high());
            break;
        }
        case 0x0B: {
            alu_.rotate_right_c(d_e_.low());
            break;
        }
        case 0x0C: {
            alu_.rotate_right_c(h_l_.high());
            break;
        }
        case 0x0D: {
            alu_.rotate_right_c(h_l_.low());
            break;
        }
        case 0x0E: {
            const auto address = make_address(h_l_);
            auto data = read_data(address);
            alu_.rotate_right_c(data);
            write_data(address, data);
            break;
        }
        case 0x0F: {
            alu_.rotate_right_c(a_f_.high());
            break;
        }
        case 0x10: {
            alu_.rotate_left(b_c_.high());
            break;
        }
        case 0x11: {
            alu_.rotate_left(b_c_.low());
            break;
        }
        case 0x12: {
            alu_.rotate_left(d_e_.high());
            break;
        }
        case 0x13: {
            alu_.rotate_left(d_e_.low());
            break;
        }
        case 0x14: {
            alu_.rotate_left(h_l_.high());
            break;
        }
        case 0x15: {
            alu_.rotate_left(h_l_.low());
            break;
        }
        case 0x16: {
            const auto address = make_address(h_l_);
            auto data = read_data(address);
            alu_.rotate_left(data);
            write_data(address, data);
            break;
        }
        case 0x17: {
            alu_.rotate_left(a_f_.high());
            break;
        }
        case 0x18: {
            alu_.rotate_right(b_c_.high());
            break;
        }
        case 0x19: {
            alu_.rotate_right(b_c_.low());
            break;
        }
        case 0x1A: {
            alu_.rotate_right(d_e_.high());
            break;
        }
        case 0x1B: {
            alu_.rotate_right(d_e_.low());
            break;
        }
        case 0x1C: {
            alu_.rotate_right(h_l_.high());
            break;
        }
        case 0x1D: {
            alu_.rotate_right(h_l_.low());
            break;
        }
        case 0x1E: {
            const auto address = make_address(h_l_);
            auto data = read_data(address);
            alu_.rotate_right(data);
            write_data(address, data);
            break;
        }
        case 0x1F: {
            alu_.rotate_right(a_f_.high());
            break;
        }
        case 0x20: {
            alu_.shift_left(b_c_.high());
            break;
        }
        case 0x21: {
            alu_.shift_left(b_c_.low());
            break;
        }
        case 0x22: {
            alu_.shift_left(d_e_.high());
            break;
        }
        case 0x23: {
            alu_.shift_left(d_e_.low());
            break;
        }
        case 0x24: {
            alu_.shift_left(h_l_.high());
            break;
        }
        case 0x25: {
            alu_.shift_left(h_l_.low());
            break;
        }
        case 0x26: {
            const auto address = make_address(h_l_);
            auto data = read_data(address);
            alu_.shift_left(data);
            write_data(address, data);
            break;
        }
        case 0x27: {
            alu_.shift_left(a_f_.high());
            break;
        }
        case 0x28: {
            alu_.shift_right(b_c_.high(), alu::preserve_last_bit);
            break;
        }
        case 0x29: {
            alu_.shift_right(b_c_.low(), alu::preserve_last_bit);
            break;
        }
        case 0x2A: {
            alu_.shift_right(d_e_.high(), alu::preserve_last_bit);
            break;
        }
        case 0x2B: {
            alu_.shift_right(d_e_.low(), alu::preserve_last_bit);
            break;
        }
        case 0x2C: {
            alu_.shift_right(h_l_.high(), alu::preserve_last_bit);
            break;
        }
        case 0x2D: {
            alu_.shift_right(h_l_.low(), alu::preserve_last_bit);
            break;
        }
        case 0x2E: {
            const auto address = make_address(h_l_);
            auto data = read_data(address);
            alu_.shift_right(data, alu::preserve_last_bit);
            write_data(address, data);
            break;
        }
        case 0x2F: {
            alu_.shift_right(a_f_.high(), alu::preserve_last_bit);
            break;
        }
        case 0x30: {
            alu_.swap(b_c_.high());
            break;
        }
        case 0x31: {
            alu_.swap(b_c_.low());
            break;
        }
        case 0x32: {
            alu_.swap(d_e_.high());
            break;
        }
        case 0x33: {
            alu_.swap(d_e_.low());
            break;
        }
        case 0x34: {
            alu_.swap(h_l_.high());
            break;
        }
        case 0x35: {
            alu_.swap(h_l_.low());
            break;
        }
        case 0x36: {
            const auto address = make_address(h_l_);
            auto data = read_data(address);
            alu_.swap(data);
            write_data(address, data);
            break;
        }
        case 0x37: {
            alu_.swap(a_f_.high());
            break;
        }
        case 0x38: {
            alu_.shift_right(b_c_.high(), alu::reset_last_bit);
            break;
        }
        case 0x39: {
            alu_.shift_right(b_c_.low(), alu::reset_last_bit);
            break;
        }
        case 0x3A: {
            alu_.shift_right(d_e_.high(), alu::reset_last_bit);
            break;
        }
        case 0x3B: {
            alu_.shift_right(d_e_.low(), alu::reset_last_bit);
            break;
        }
        case 0x3C: {
            alu_.shift_right(h_l_.high(), alu::reset_last_bit);
            break;
        }
        case 0x3D: {
            alu_.shift_right(h_l_.low(), alu::reset_last_bit);
            break;
        }
        case 0x3E: {
            const auto address = make_address(h_l_);
            auto data = read_data(address);
            alu_.shift_right(data, alu::reset_last_bit);
            write_data(address, data);
            break;
        }
        case 0x3F: {
            alu_.shift_right(a_f_.high(), alu::reset_last_bit);
            break;
        }

        case 0x40: {
            alu_.test(b_c_.high(), 0);
            break;
        }
        case 0x41: {
            alu_.test(b_c_.low(), 0);
            break;
        }
        case 0x42: {
            alu_.test(d_e_.high(), 0);
            break;
        }
        case 0x43: {
            alu_.test(d_e_.low(), 0);
            break;
        }
        case 0x44: {
            alu_.test(h_l_.high(), 0);
            break;
        }
        case 0x45: {
            alu_.test(h_l_.low(), 0);
            break;
        }
        case 0x46: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu_.test(data, 0);
            write_data(addr, data);
            break;
        }
        case 0x47: {
            alu_.test(a_f_.high(), 0);
            break;
        }

        case 0x48: {
            alu_.test(b_c_.high(), 1);
            break;
        }
        case 0x49: {
            alu_.test(b_c_.low(), 1);
            break;
        }
        case 0x4A: {
            alu_.test(d_e_.high(), 1);
            break;
        }
        case 0x4B: {
            alu_.test(d_e_.low(), 1);
            break;
        }
        case 0x4C: {
            alu_.test(h_l_.high(), 1);
            break;
        }
        case 0x4D: {
            alu_.test(h_l_.low(), 1);
            break;
        }
        case 0x4E: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu_.test(data, 1);
            write_data(addr, data);
            break;
        }
        case 0x4F: {
            alu_.test(a_f_.high(), 1);
            break;
        }

        case 0x50: {
            alu_.test(b_c_.high(), 2);
            break;
        }
        case 0x51: {
            alu_.test(b_c_.low(), 2);
            break;
        }
        case 0x52: {
            alu_.test(d_e_.high(), 2);
            break;
        }
        case 0x53: {
            alu_.test(d_e_.low(), 2);
            break;
        }
        case 0x54: {
            alu_.test(h_l_.high(), 2);
            break;
        }
        case 0x55: {
            alu_.test(h_l_.low(), 2);
            break;
        }
        case 0x56: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu_.test(data, 2);
            write_data(addr, data);
            break;
        }
        case 0x57: {
            alu_.test(a_f_.high(), 2);
            break;
        }

        case 0x58: {
            alu_.test(b_c_.high(), 3);
            break;
        }
        case 0x59: {
            alu_.test(b_c_.low(), 3);
            break;
        }
        case 0x5A: {
            alu_.test(d_e_.high(), 3);
            break;
        }
        case 0x5B: {
            alu_.test(d_e_.low(), 3);
            break;
        }
        case 0x5C: {
            alu_.test(h_l_.high(), 3);
            break;
        }
        case 0x5D: {
            alu_.test(h_l_.low(), 3);
            break;
        }
        case 0x5E: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu_.test(data, 3);
            write_data(addr, data);
            break;
        }
        case 0x5F: {
            alu_.test(a_f_.high(), 3);
            break;
        }

        case 0x60: {
            alu_.test(b_c_.high(), 4);
            break;
        }
        case 0x61: {
            alu_.test(b_c_.low(), 4);
            break;
        }
        case 0x62: {
            alu_.test(d_e_.high(), 4);
            break;
        }
        case 0x63: {
            alu_.test(d_e_.low(), 4);
            break;
        }
        case 0x64: {
            alu_.test(h_l_.high(), 4);
            break;
        }
        case 0x65: {
            alu_.test(h_l_.low(), 4);
            break;
        }
        case 0x66: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu_.test(data, 4);
            write_data(addr, data);
            break;
        }
        case 0x67: {
            alu_.test(a_f_.high(), 4);
            break;
        }

        case 0x68: {
            alu_.test(b_c_.high(), 5);
            break;
        }
        case 0x69: {
            alu_.test(b_c_.low(), 5);
            break;
        }
        case 0x6A: {
            alu_.test(d_e_.high(), 5);
            break;
        }
        case 0x6B: {
            alu_.test(d_e_.low(), 5);
            break;
        }
        case 0x6C: {
            alu_.test(h_l_.high(), 5);
            break;
        }
        case 0x6D: {
            alu_.test(h_l_.low(), 5);
            break;
        }
        case 0x6E: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu_.test(data, 5);
            write_data(addr, data);
            break;
        }
        case 0x6F: {
            alu_.test(a_f_.high(), 5);
            break;
        }

        case 0x70: {
            alu_.test(b_c_.high(), 6);
            break;
        }
        case 0x71: {
            alu_.test(b_c_.low(), 6);
            break;
        }
        case 0x72: {
            alu_.test(d_e_.high(), 6);
            break;
        }
        case 0x73: {
            alu_.test(d_e_.low(), 6);
            break;
        }
        case 0x74: {
            alu_.test(h_l_.high(), 6);
            break;
        }
        case 0x75: {
            alu_.test(h_l_.low(), 6);
            break;
        }
        case 0x76: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu_.test(data, 6);
            write_data(addr, data);
            break;
        }
        case 0x77: {
            alu_.test(a_f_.high(), 6);
            break;
        }

        case 0x78: {
            alu_.test(b_c_.high(), 7);
            break;
        }
        case 0x79: {
            alu_.test(b_c_.low(), 7);
            break;
        }
        case 0x7A: {
            alu_.test(d_e_.high(), 7);
            break;
        }
        case 0x7B: {
            alu_.test(d_e_.low(), 7);
            break;
        }
        case 0x7C: {
            alu_.test(h_l_.high(), 7);
            break;
        }
        case 0x7D: {
            alu_.test(h_l_.low(), 7);
            break;
        }
        case 0x7E: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu_.test(data, 7);
            write_data(addr, data);
            break;
        }
        case 0x7F: {
            alu_.test(a_f_.high(), 7);
            break;
        }

        case 0x80: {
            alu_.reset(b_c_.high(), 0);
            break;
        }
        case 0x81: {
            alu_.reset(b_c_.low(), 0);
            break;
        }
        case 0x82: {
            alu_.reset(d_e_.high(), 0);
            break;
        }
        case 0x83: {
            alu_.reset(d_e_.low(), 0);
            break;
        }
        case 0x84: {
            alu_.reset(h_l_.high(), 0);
            break;
        }
        case 0x85: {
            alu_.reset(h_l_.low(), 0);
            break;
        }
        case 0x86: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu::reset(data, 0);
            write_data(addr, data);
            break;
        }
        case 0x87: {
            alu_.reset(a_f_.high(), 0);
            break;
        }

        case 0x88: {
            alu_.reset(b_c_.high(), 1);
            break;
        }
        case 0x89: {
            alu_.reset(b_c_.low(), 1);
            break;
        }
        case 0x8A: {
            alu_.reset(d_e_.high(), 1);
            break;
        }
        case 0x8B: {
            alu_.reset(d_e_.low(), 1);
            break;
        }
        case 0x8C: {
            alu_.reset(h_l_.high(), 1);
            break;
        }
        case 0x8D: {
            alu_.reset(h_l_.low(), 1);
            break;
        }
        case 0x8E: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu::reset(data, 1);
            write_data(addr, data);
            break;
        }
        case 0x8F: {
            alu_.reset(a_f_.high(), 1);
            break;
        }

        case 0x90: {
            alu_.reset(b_c_.high(), 2);
            break;
        }
        case 0x91: {
            alu_.reset(b_c_.low(), 2);
            break;
        }
        case 0x92: {
            alu_.reset(d_e_.high(), 2);
            break;
        }
        case 0x93: {
            alu_.reset(d_e_.low(), 2);
            break;
        }
        case 0x94: {
            alu_.reset(h_l_.high(), 2);
            break;
        }
        case 0x95: {
            alu_.reset(h_l_.low(), 2);
            break;
        }
        case 0x96: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu::reset(data, 2);
            write_data(addr, data);
            break;
        }
        case 0x97: {
            alu_.reset(a_f_.high(), 2);
            break;
        }

        case 0x98: {
            alu_.reset(b_c_.high(), 3);
            break;
        }
        case 0x99: {
            alu_.reset(b_c_.low(), 3);
            break;
        }
        case 0x9A: {
            alu_.reset(d_e_.high(), 3);
            break;
        }
        case 0x9B: {
            alu_.reset(d_e_.low(), 3);
            break;
        }
        case 0x9C: {
            alu_.reset(h_l_.high(), 3);
            break;
        }
        case 0x9D: {
            alu_.reset(h_l_.low(), 3);
            break;
        }
        case 0x9E: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu::reset(data, 3);
            write_data(addr, data);
            break;
        }
        case 0x9F: {
            alu_.reset(a_f_.high(), 3);
            break;
        }

        case 0xA0: {
            alu_.reset(b_c_.high(), 4);
            break;
        }
        case 0xA1: {
            alu_.reset(b_c_.low(), 4);
            break;
        }
        case 0xA2: {
            alu_.reset(d_e_.high(), 4);
            break;
        }
        case 0xA3: {
            alu_.reset(d_e_.low(), 4);
            break;
        }
        case 0xA4: {
            alu_.reset(h_l_.high(), 4);
            break;
        }
        case 0xA5: {
            alu_.reset(h_l_.low(), 4);
            break;
        }
        case 0xA6: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu::reset(data, 4);
            write_data(addr, data);
            break;
        }
        case 0xA7: {
            alu_.reset(a_f_.high(), 4);
            break;
        }

        case 0xA8: {
            alu_.reset(b_c_.high(), 5);
            break;
        }
        case 0xA9: {
            alu_.reset(b_c_.low(), 5);
            break;
        }
        case 0xAA: {
            alu_.reset(d_e_.high(), 5);
            break;
        }
        case 0xAB: {
            alu_.reset(d_e_.low(), 5);
            break;
        }
        case 0xAC: {
            alu_.reset(h_l_.high(), 5);
            break;
        }
        case 0xAD: {
            alu_.reset(h_l_.low(), 5);
            break;
        }
        case 0xAE: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu::reset(data, 5);
            write_data(addr, data);
            break;
        }
        case 0xAF: {
            alu_.reset(a_f_.high(), 5);
            break;
        }

        case 0xB0: {
            alu_.reset(b_c_.high(), 6);
            break;
        }
        case 0xB1: {
            alu_.reset(b_c_.low(), 6);
            break;
        }
        case 0xB2: {
            alu_.reset(d_e_.high(), 6);
            break;
        }
        case 0xB3: {
            alu_.reset(d_e_.low(), 6);
            break;
        }
        case 0xB4: {
            alu_.reset(h_l_.high(), 6);
            break;
        }
        case 0xB5: {
            alu_.reset(h_l_.low(), 6);
            break;
        }
        case 0xB6: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu::reset(data, 6);
            write_data(addr, data);
            break;
        }
        case 0xB7: {
            alu_.reset(a_f_.high(), 6);
            break;
        }

        case 0xB8: {
            alu_.reset(b_c_.high(), 7);
            break;
        }
        case 0xB9: {
            alu_.reset(b_c_.low(), 7);
            break;
        }
        case 0xBA: {
            alu_.reset(d_e_.high(), 7);
            break;
        }
        case 0xBB: {
            alu_.reset(d_e_.low(), 7);
            break;
        }
        case 0xBC: {
            alu_.reset(h_l_.high(), 7);
            break;
        }
        case 0xBD: {
            alu_.reset(h_l_.low(), 7);
            break;
        }
        case 0xBE: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu::reset(data, 7);
            write_data(addr, data);
            break;
        }
        case 0xBF: {
            alu_.reset(a_f_.high(), 7);
            break;
        }

        case 0xC0: {
            alu_.set(b_c_.high(), 0);
            break;
        }
        case 0xC1: {
            alu_.set(b_c_.low(), 0);
            break;
        }
        case 0xC2: {
            alu_.set(d_e_.high(), 0);
            break;
        }
        case 0xC3: {
            alu_.set(d_e_.low(), 0);
            break;
        }
        case 0xC4: {
            alu_.set(h_l_.high(), 0);
            break;
        }
        case 0xC5: {
            alu_.set(h_l_.low(), 0);
            break;
        }
        case 0xC6: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu::set(data, 0);
            write_data(addr, data);
            break;
        }
        case 0xC7: {
            alu_.set(a_f_.high(), 0);
            break;
        }

        case 0xC8: {
            alu_.set(b_c_.high(), 1);
            break;
        }
        case 0xC9: {
            alu_.set(b_c_.low(), 1);
            break;
        }
        case 0xCA: {
            alu_.set(d_e_.high(), 1);
            break;
        }
        case 0xCB: {
            alu_.set(d_e_.low(), 1);
            break;
        }
        case 0xCC: {
            alu_.set(h_l_.high(), 1);
            break;
        }
        case 0xCD: {
            alu_.set(h_l_.low(), 1);
            break;
        }
        case 0xCE: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu::set(data, 1);
            write_data(addr, data);
            break;
        }
        case 0xCF: {
            alu_.set(a_f_.high(), 1);
            break;
        }

        case 0xD0: {
            alu_.set(b_c_.high(), 2);
            break;
        }
        case 0xD1: {
            alu_.set(b_c_.low(), 2);
            break;
        }
        case 0xD2: {
            alu_.set(d_e_.high(), 2);
            break;
        }
        case 0xD3: {
            alu_.set(d_e_.low(), 2);
            break;
        }
        case 0xD4: {
            alu_.set(h_l_.high(), 2);
            break;
        }
        case 0xD5: {
            alu_.set(h_l_.low(), 2);
            break;
        }
        case 0xD6: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu::set(data, 2);
            write_data(addr, data);
            break;
        }
        case 0xD7: {
            alu_.set(a_f_.high(), 2);
            break;
        }

        case 0xD8: {
            alu_.set(b_c_.high(), 3);
            break;
        }
        case 0xD9: {
            alu_.set(b_c_.low(), 3);
            break;
        }
        case 0xDA: {
            alu_.set(d_e_.high(), 3);
            break;
        }
        case 0xDB: {
            alu_.set(d_e_.low(), 3);
            break;
        }
        case 0xDC: {
            alu_.set(h_l_.high(), 3);
            break;
        }
        case 0xDD: {
            alu_.set(h_l_.low(), 3);
            break;
        }
        case 0xDE: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu::set(data, 3);
            write_data(addr, data);
            break;
        }
        case 0xDF: {
            alu_.set(a_f_.high(), 3);
            break;
        }

        case 0xE0: {
            alu_.set(b_c_.high(), 4);
            break;
        }
        case 0xE1: {
            alu_.set(b_c_.low(), 4);
            break;
        }
        case 0xE2: {
            alu_.set(d_e_.high(), 4);
            break;
        }
        case 0xE3: {
            alu_.set(d_e_.low(), 4);
            break;
        }
        case 0xE4: {
            alu_.set(h_l_.high(), 4);
            break;
        }
        case 0xE5: {
            alu_.set(h_l_.low(), 4);
            break;
        }
        case 0xE6: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu::set(data, 4);
            write_data(addr, data);
            break;
        }
        case 0xE7: {
            alu_.set(a_f_.high(), 4);
            break;
        }

        case 0xE8: {
            alu_.set(b_c_.high(), 5);
            break;
        }
        case 0xE9: {
            alu_.set(b_c_.low(), 5);
            break;
        }
        case 0xEA: {
            alu_.set(d_e_.high(), 5);
            break;
        }
        case 0xEB: {
            alu_.set(d_e_.low(), 5);
            break;
        }
        case 0xEC: {
            alu_.set(h_l_.high(), 5);
            break;
        }
        case 0xED: {
            alu_.set(h_l_.low(), 5);
            break;
        }
        case 0xEE: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu::set(data, 5);
            write_data(addr, data);
            break;
        }
        case 0xEF: {
            alu_.set(a_f_.high(), 5);
            break;
        }

        case 0xF0: {
            alu_.set(b_c_.high(), 6);
            break;
        }
        case 0xF1: {
            alu_.set(b_c_.low(), 6);
            break;
        }
        case 0xF2: {
            alu_.set(d_e_.high(), 6);
            break;
        }
        case 0xF3: {
            alu_.set(d_e_.low(), 6);
            break;
        }
        case 0xF4: {
            alu_.set(h_l_.high(), 6);
            break;
        }
        case 0xF5: {
            alu_.set(h_l_.low(), 6);
            break;
        }
        case 0xF6: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu::set(data, 6);
            write_data(addr, data);
            break;
        }
        case 0xF7: {
            alu_.set(a_f_.high(), 6);
            break;
        }

        case 0xF8: {
            alu_.set(b_c_.high(), 7);
            break;
        }
        case 0xF9: {
            alu_.set(b_c_.low(), 7);
            break;
        }
        case 0xFA: {
            alu_.set(d_e_.high(), 7);
            break;
        }
        case 0xFB: {
            alu_.set(d_e_.low(), 7);
            break;
        }
        case 0xFC: {
            alu_.set(h_l_.high(), 7);
            break;
        }
        case 0xFD: {
            alu_.set(h_l_.low(), 7);
            break;
        }
        case 0xFE: {
            const auto addr = make_address(h_l_);
            auto data = read_data(addr);
            alu::set(data, 7);
            write_data(addr, data);
            break;
        }
        case 0xFF: {
            alu_.set(a_f_.high(), 7);
            break;
        }

        default: {
            spdlog::critical("unknown instruction: {:#x}, address: {:#x}", inst, program_counter_.value() - 1);
        }
    }

    const auto& info = instruction::extended_instruction_set[inst];

#if WITH_DEBUGGER
    if(on_instruction_executed_) {
        on_instruction_executed_(make_address(prev_program_counter_), info, 0u);
    }
#endif //WITH_DEBUGGER

    return info.cycle_count;
}

void cpu::write_data(const address16& address, const uint8_t data)
{
    bus_->get_mmu()->write(address, data);
}

uint8_t cpu::read_data(const address16& address) const
{
    return bus_->get_mmu()->read(address);
}

uint8_t cpu::read_immediate(imm8_t)
{
    return read_data(make_address(program_counter_++));
}

uint16_t cpu::read_immediate(imm16_t)
{
    const auto lsb = read_immediate(imm8);
    const auto msb = read_immediate(imm8);

    return word(msb, lsb);
}

void cpu::nop() noexcept {}

void cpu::halt() noexcept
{
    if(pending_enable_interrupts_counter_ != -1) {
        interrupt_master_enable_ = true;
        pending_enable_interrupts_counter_ = -1;
        --program_counter_;
    } else if(pending_disable_interrupts_counter_ != -1) {
        interrupt_master_enable_ = false;
        pending_disable_interrupts_counter_ = -1;
        --program_counter_;
    } else {
        is_halted_ = true;
    }
}

void cpu::stop() noexcept
{
    if(bit::test(key_1_, 0u)) {
        key_1_ = bit::reset(key_1_, 0u);
        key_1_ = bit::flip(key_1_, 7u);
        return;
    }

    is_stopped_ = true;
    is_halted_ = true;
}

void cpu::push(const register16& reg)
{
    write_data(make_address(--stack_pointer_), reg.high().value());
    write_data(make_address(--stack_pointer_), reg.low().value());
}

void cpu::pop(register16& reg)
{
    reg.low() = read_data(make_address(stack_pointer_++));
    reg.high() = read_data(make_address(stack_pointer_++));
}

void cpu::rst(const address8& address)
{
    push(program_counter_);
    program_counter_ = address;
}

void cpu::jump(const register16& reg)
{
    program_counter_ = reg;
}

void cpu::jump(const address16& address)
{
    program_counter_ = address;
}

void cpu::jump_relative(const address8& address) noexcept
{
    const auto signed_addr = static_cast<int8_t>(address.value());
    program_counter_ = static_cast<uint16_t>(program_counter_.value() + signed_addr);
}

void cpu::call(const address16& address)
{
    push(program_counter_);
    program_counter_ = address;
}

void cpu::reti()
{
    ret();
    pending_disable_interrupts_counter_ = -1;
    pending_enable_interrupts_counter_ = -1;
    interrupt_master_enable_ = true;
}

void cpu::ret()
{
    pop(program_counter_);
}

void cpu::store(const address16& address, const uint8_t data)
{
    write_data(address, data);
}

void cpu::store(const address16& address, const register8& reg)
{
    write_data(address, reg.value());
}

void cpu::store(const address16& address, const register16& reg)
{
    store(address, reg.low());
    store(address + 1, reg.high());
}

void cpu::load(register8& reg, const uint8_t data) noexcept
{
    reg = data;
}

void cpu::load(register8& r_left, const register8& r_right) noexcept
{
    r_left = r_right;
}

void cpu::load(register16& reg, const uint16_t data) noexcept
{
    reg = data;
}

void cpu::load(register16& r_left, const register16& r_right) noexcept
{
    r_left = r_right;
}

void cpu::load_hlsp(const int8_t data) noexcept
{
    const auto value = stack_pointer_.value() + data;

    reset_flag(flag::all);
    if(mask::test(stack_pointer_.value() ^ data ^ (value & 0xFFFF), 0x0100)) {
        set_flag(flag::carry);
    }

    if(mask::test(stack_pointer_.value() ^ data ^ (value & 0xFFFF), 0x0010)) {
        set_flag(flag::half_carry);
    }

    load(h_l_, static_cast<uint16_t>(value));
}

} // namespace gameboy
