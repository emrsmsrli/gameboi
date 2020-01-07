#include <array>
#include <magic_enum.hpp>

#include "gameboy/cpu/cpu.h"
#include "gameboy/cpu/instruction_info.h"
#include "gameboy/bus.h"
#include "gameboy/memory/mmu.h"
#include "gameboy/memory/address.h"
#include "gameboy/util/log.h"
#include "gameboy/util/observer.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {
    
using namespace magic_enum::bitwise_operators;

constexpr address16 ie_addr{0xFFFFu};
constexpr address16 if_addr{0xFF0Fu};

cpu::cpu(observer<bus> bus) noexcept
    : bus_{bus},
      alu_{make_observer(this)},
      a_f_{0x01B0u},
      b_c_{0x0013u},
      d_e_{0x00D8u},
      h_l_{0x014Du},
      stack_pointer_{0xFFFEu},
      program_counter_{0x0100u},
      total_cycles_{0u},
      interrupt_flags_{interrupt::none},
      interrupt_enable_{interrupt::none},
      interrupt_master_enable_{false},
      is_interrupt_status_change_pending_{false},
      is_halted_{false},
      is_halt_bug_triggered_{false}
{
    auto mmu = bus->get_mmu();

    mmu->add_memory_delegate({
        ie_addr,
        {connect_arg<&cpu::on_ie_read>, this},
        {connect_arg<&cpu::on_ie_write>, this},
    });

    mmu->add_memory_delegate({
        if_addr,
        {connect_arg<&cpu::on_if_read>, this},
        {connect_arg<&cpu::on_if_write>, this},
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
    return static_cast<uint8_t>(interrupt_flags_);
}

uint8_t cpu::tick()
{
    const auto execute_next_op = [&]() -> uint8_t {
        const auto opcode = read_immediate(imm8);
        if(opcode != 0xCB) {
            return decode(opcode, standard_instruction_set);
        }

        return decode(read_immediate(imm8), extended_instruction_set);
    };

    const auto cycle_count =
        !is_halted_
        ? execute_next_op()
        : static_cast<uint8_t>(0x1u);

    // todo ime should be true or false AFTER one instruction is executed after EI or DI instruction
    // todo check_power_mode();

    if(interrupt_master_enable_) {
        const auto pending = interrupt_enable_ & interrupt_flags_;

        const auto interrupt_requested = [&](const interrupt i) {
            return (pending & i) != interrupt::none;
        };

        const auto do_interrupt = [&](const interrupt i) {
            interrupt_master_enable_ = false;
            interrupt_flags_ &= ~i;
            rst(make_address(i));
        };

        static constexpr std::array interrupts = {
            interrupt::joypad,
            interrupt::serial,
            interrupt::timer,
            interrupt::lcd_stat,
            interrupt::lcd_vblank
        };
        for(auto i : interrupts) {
            if(interrupt_requested(i)) {
                do_interrupt(i);
            }
        }
    }

    total_cycles_ += cycle_count;
    return cycle_count;
}

void cpu::request_interrupt(const interrupt request) noexcept
{
    interrupt_flags_ |= request;
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
    return mask_test(a_f_.low(), f);
}

uint8_t cpu::decode(const uint16_t inst, standard_instruction_set_t)
{
    const auto info = instruction::standard_instruction_set[inst];

    const auto data = [&]() -> uint16_t {
        switch(info.length) {
            case 3:return read_immediate(imm16);
            case 2:return read_immediate(imm8);
            default:return 0u;
        }
    }();

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
                return instruction::get_false_branch_cycle_count(inst);
            }
            break;
        }
        case 0x21: {
            load(h_l_, data);
            break;
        }
        case 0x22: {
            store_i();
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
                return instruction::get_false_branch_cycle_count(inst);
            }
            break;
        }
        case 0x29: {
            alu_.add(h_l_, h_l_);
            break;
        }
        case 0x2A: {
            load_i();
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
                return instruction::get_false_branch_cycle_count(inst);
            }
            break;
        }
        case 0x31: {
            load(stack_pointer_, data);
            break;
        }
        case 0x32: {
            store_d();
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
            reset_flag(flag::negative);
            reset_flag(flag::half_carry);
            set_flag(flag::carry);
            break;
        }
        case 0x38: {
            if(test_flag(flag::carry)) {
                jump_relative(make_address(static_cast<uint8_t>(data)));
            } else {
                return instruction::get_false_branch_cycle_count(inst);
            }
            break;
        }
        case 0x39: {
            alu_.add(h_l_, stack_pointer_);
            break;
        }
        case 0x3A: {
            load_d();
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
            reset_flag(flag::negative);
            reset_flag(flag::half_carry);
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
            load(h_l_.low(), h_l_.low());
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
            /* AND A */
            nop();
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
            const auto data = read_data(make_address(h_l_));
            alu_.logical_compare(data);
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
                return instruction::get_false_branch_cycle_count(inst);
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
                return instruction::get_false_branch_cycle_count(inst);
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
                return instruction::get_false_branch_cycle_count(inst);
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
                return instruction::get_false_branch_cycle_count(inst);
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
                return instruction::get_false_branch_cycle_count(inst);
            }
            break;
        }
        case 0xCC: {
            if(test_flag(flag::zero)) {
                call(make_address(data));
            } else {
                return instruction::get_false_branch_cycle_count(inst);
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
                return instruction::get_false_branch_cycle_count(inst);
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
                return instruction::get_false_branch_cycle_count(inst);
            }
            break;
        }
        case 0xD4: {
            if(!test_flag(flag::carry)) {
                call(make_address(data));
            } else {
                return instruction::get_false_branch_cycle_count(inst);
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
                return instruction::get_false_branch_cycle_count(inst);
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
                return instruction::get_false_branch_cycle_count(inst);
            }
            break;
        }
        case 0xDC: {
            if(test_flag(flag::carry)) {
                call(make_address(data));
            } else {
                return instruction::get_false_branch_cycle_count(inst);
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
            break;
        }
        case 0xF2: {
            load(a_f_.high(), read_data(make_address(b_c_.low() + 0xFF00)));
            break;
        }
        case 0xF3: {
            interrupt_master_enable_ = false;
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
            load_hlsp();
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
            interrupt_master_enable_ = true;
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
            log::error("unknown instruction: {:#x}, address: {:#x}", inst, stack_pointer_.value() - 1);
        }
    }

    return info.cycle_count;
}

uint8_t cpu::decode(uint16_t inst, extended_instruction_set_t)
{
    const auto info = instruction::extended_instruction_set[inst];

    const auto get_bitop_mask = [&]() -> uint8_t {
        return 0x1u << (inst >> 0x3u & 0x7u);
    };

    const auto alu_do_one_param = [&](void (alu::*func)(uint8_t&) const) {
        const auto address = make_address(h_l_);
        auto data = read_data(address);
        std::invoke(func, alu_, data);
        write_data(address, data);
    };

    const auto alu_do_two_params = [&](void (alu::*func)(uint8_t&) const, auto&& param) {
        const auto address = make_address(h_l_);
        auto data = read_data(address);
        std::invoke(func, alu_, data, std::forward<decltype(param)>(param));
        write_data(address, data);
    };

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
            alu_do_one_param(&alu::rotate_left_c);
            break;
        }
        case 0x07: {
            alu_.rotate_left_c(b_c_.high());
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
            alu_do_one_param(&alu::rotate_right_c);
            break;
        }
        case 0x0F: {
            alu_.rotate_right_c(b_c_.high());
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
            alu_do_one_param(&alu::rotate_left);
            break;
        }
        case 0x17: {
            alu_.rotate_left(b_c_.high());
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
            alu_do_one_param(&alu::rotate_right);
            break;
        }
        case 0x1F: {
            alu_.rotate_right(b_c_.high());
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
            alu_do_one_param(&alu::shift_left);
            break;
        }
        case 0x27: {
            alu_.shift_left(b_c_.high());
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
            alu_.shift_right(b_c_.high(), alu::preserve_last_bit);
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
            alu_do_one_param(&alu::swap);
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

        case 0x40:
        case 0x50:
        case 0x60:
        case 0x70:
        case 0x48:
        case 0x58:
        case 0x68:
        case 0x78: {
            alu_.test(b_c_.high(), get_bitop_mask());
            break;
        }

        case 0x41:
        case 0x51:
        case 0x61:
        case 0x71:
        case 0x49:
        case 0x59:
        case 0x69:
        case 0x79: {
            alu_.test(b_c_.low(), get_bitop_mask());
            break;
        }

        case 0x42:
        case 0x52:
        case 0x62:
        case 0x72:
        case 0x4A:
        case 0x5A:
        case 0x6A:
        case 0x7A: {
            alu_.test(d_e_.high(), get_bitop_mask());
            break;
        }

        case 0x43:
        case 0x53:
        case 0x63:
        case 0x73:
        case 0x4B:
        case 0x5B:
        case 0x6B:
        case 0x7B: {
            alu_.test(d_e_.low(), get_bitop_mask());
            break;
        }

        case 0x44:
        case 0x54:
        case 0x64:
        case 0x74:
        case 0x4C:
        case 0x5C:
        case 0x6C:
        case 0x7C: {
            alu_.test(h_l_.high(), get_bitop_mask());
            break;
        }

        case 0x45:
        case 0x55:
        case 0x65:
        case 0x75:
        case 0x4D:
        case 0x5D:
        case 0x6D:
        case 0x7D: {
            alu_.test(h_l_.low(), get_bitop_mask());
            break;
        }

        case 0x46:
        case 0x56:
        case 0x66:
        case 0x76:
        case 0x4E:
        case 0x5E:
        case 0x6E:
        case 0x7E: {
            const auto data = read_data(make_address(h_l_));
            alu_.test(data, get_bitop_mask());
            break;
        }

        case 0x47:
        case 0x57:
        case 0x67:
        case 0x77:
        case 0x4F:
        case 0x5F:
        case 0x6F:
        case 0x7F: {
            alu_.test(a_f_.high(), get_bitop_mask());
            break;
        }

        case 0x80:
        case 0x90:
        case 0xA0:
        case 0xB0:
        case 0x88:
        case 0x98:
        case 0xA8:
        case 0xB8: {
            alu_.reset(b_c_.high(), get_bitop_mask());
            break;
        }

        case 0x81:
        case 0x91:
        case 0xA1:
        case 0xB1:
        case 0x89:
        case 0x99:
        case 0xA9:
        case 0xB9: {
            alu_.reset(b_c_.low(), get_bitop_mask());
            break;
        }

        case 0x82:
        case 0x92:
        case 0xA2:
        case 0xB2:
        case 0x8A:
        case 0x9A:
        case 0xAA:
        case 0xBA: {
            alu_.reset(d_e_.high(), get_bitop_mask());
            break;
        }

        case 0x83:
        case 0x93:
        case 0xA3:
        case 0xB3:
        case 0x8B:
        case 0x9B:
        case 0xAB:
        case 0xBB: {
            alu_.reset(d_e_.low(), get_bitop_mask());
            break;
        }

        case 0x84:
        case 0x94:
        case 0xA4:
        case 0xB4:
        case 0x8C:
        case 0x9C:
        case 0xAC:
        case 0xBC: {
            alu_.reset(h_l_.high(), get_bitop_mask());
            break;
        }

        case 0x85:
        case 0x95:
        case 0xA5:
        case 0xB5:
        case 0x8D:
        case 0x9D:
        case 0xAD:
        case 0xBD: {
            alu_.reset(h_l_.low(), get_bitop_mask());
            break;
        }

        case 0x86:
        case 0x96:
        case 0xA6:
        case 0xB6:
        case 0x8E:
        case 0x9E:
        case 0xAE:
        case 0xBE: {
            const auto address = make_address(h_l_);
            auto data = read_data(address);
            alu::reset(data, get_bitop_mask());
            write_data(address, data);
            break;
        }

        case 0x87:
        case 0x97:
        case 0xA7:
        case 0xB7:
        case 0x8F:
        case 0x9F:
        case 0xAF:
        case 0xBF: {
            alu_.reset(a_f_.high(), get_bitop_mask());
            break;
        }

        case 0xC0:
        case 0xD0:
        case 0xE0:
        case 0xF0:
        case 0xC8:
        case 0xD8:
        case 0xE8:
        case 0xF8: {
            alu_.set(b_c_.high(), get_bitop_mask());
            break;
        }

        case 0xC1:
        case 0xD1:
        case 0xE1:
        case 0xF1:
        case 0xC9:
        case 0xD9:
        case 0xE9:
        case 0xF9: {
            alu_.set(b_c_.low(), get_bitop_mask());
            break;
        }

        case 0xC2:
        case 0xD2:
        case 0xE2:
        case 0xF2:
        case 0xCA:
        case 0xDA:
        case 0xEA:
        case 0xFA: {
            alu_.set(d_e_.high(), get_bitop_mask());
            break;
        }

        case 0xC3:
        case 0xD3:
        case 0xE3:
        case 0xF3:
        case 0xCB:
        case 0xDB:
        case 0xEB:
        case 0xFB: {
            alu_.set(d_e_.low(), get_bitop_mask());
            break;
        }

        case 0xC4:
        case 0xD4:
        case 0xE4:
        case 0xF4:
        case 0xCC:
        case 0xDC:
        case 0xEC:
        case 0xFC: {
            alu_.set(h_l_.high(), get_bitop_mask());
            break;
        }

        case 0xC5:
        case 0xD5:
        case 0xE5:
        case 0xF5:
        case 0xCD:
        case 0xDD:
        case 0xED:
        case 0xFD: {
            alu_.set(h_l_.low(), get_bitop_mask());
            break;
        }

        case 0xC6:
        case 0xD6:
        case 0xE6:
        case 0xF6:
        case 0xCE:
        case 0xDE:
        case 0xEE:
        case 0xFE: {
            const auto address = make_address(h_l_);
            auto data = read_data(address);
            alu::set(data, get_bitop_mask());
            write_data(address, data);
            break;
        }

        case 0xC7:
        case 0xD7:
        case 0xE7:
        case 0xF7:
        case 0xCF:
        case 0xDF:
        case 0xEF:
        case 0xFF: {
            alu_.set(a_f_.high(), get_bitop_mask());
            break;
        }

        default: {
            log::error("unknown instruction: {:#x}, address: {:#x}", inst, stack_pointer_.value() - 1);
        }
    }

    return instruction::extended_instruction_set[inst].cycle_count;
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
    const auto data = read_data(make_address(program_counter_));
    ++program_counter_;
    return data;
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
    is_halted_ = true;

    // Check halt bug
    // byte interruptEnabledflag = memory->read(IE_REGISTER);
    // byte interruptflag = memory->read(IF_REGISTER);

    // todo investigate this
    // if(!is_interrupt_master_enabled && (interruptflag & interruptEnabledflag & 0x1F)) {
    //     is_halt_bug_triggered = true;
    // }
}

void cpu::stop() noexcept
{
    // todo make this a separate instruction to save energy by turning off the system completely
    halt();
}

void cpu::push(const register16& reg)
{
    const auto write_to_stack = [&](const register8& reg_8) {
        --stack_pointer_;
        write_data(make_address(stack_pointer_), reg_8.value());
    };

    write_to_stack(reg.high());
    write_to_stack(reg.low());
}

void cpu::pop(register16& reg)
{
    const auto read_from_stack = [&]() {
        const auto data = read_data(make_address(stack_pointer_));
        ++stack_pointer_;
        return data;
    };

    reg.low() = read_from_stack();
    reg.high() = read_from_stack();
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
    program_counter_ += address;
}

void cpu::call(const address16& address)
{
    push(program_counter_);
    program_counter_ = address;
}

void cpu::reti()
{
    interrupt_master_enable_ = true;
    ret();
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
    store(address, reg.high());
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

void cpu::store_i() noexcept
{
    store(make_address(h_l_), a_f_.high());
    ++h_l_;
}

void cpu::store_d() noexcept
{
    store(make_address(h_l_), a_f_.high());
    --h_l_;
}

void cpu::load_i() noexcept
{
    const auto data = read_data(make_address(h_l_));
    load(a_f_.high(), data);
    ++h_l_;
}

void cpu::load_d() noexcept
{
    const auto data = read_data(make_address(h_l_));
    load(a_f_.high(), data);
    --h_l_;
}

void cpu::load_hlsp() noexcept
{
    const auto data = static_cast<int8_t>(read_immediate(imm8));
    const uint16_t value = stack_pointer_.value() + data;

    reset_flag(flag::all);
    if(((stack_pointer_.value() ^ data ^ value) & 0x0100) == 0x0100) {
        set_flag(flag::carry);
    }

    if(((stack_pointer_.value() ^ data ^ value) & 0x0010) == 0x0010) {
        set_flag(flag::half_carry);
    }

    load(h_l_, value);
}

} // namespace gameboy
