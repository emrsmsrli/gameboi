#include <array>

#include <cpu/cpu.h>
#include <bus.h>
#include <memory/mmu.h>
#include <memory/address.h>
#include <util/log.h>
#include <util/observer.h>
#include <util/mathutil.h>

namespace gameboy {

constexpr address16 ime_addr(0xFFFFu);

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
        ime_addr,
        {connect_arg<&cpu::on_ie_read>, this},
        {connect_arg<&cpu::on_ie_write>, this},
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

        const auto interrupt_requested = [&](interrupt i) {
            return (pending & i) != interrupt::none;
        };

        const auto do_interrupt = [&](interrupt i) {
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
    interrupt_master_enable_ = true;
    interrupt_flags_ |= request;
}

void cpu::set_flag(flag flag) noexcept
{
    a_f_.low() |= static_cast<uint8_t>(flag);
}

void cpu::reset_flag(flag flag) noexcept
{
    a_f_.low() &= ~static_cast<uint8_t>(flag);
}

void cpu::flip_flag(flag flag) noexcept
{
    a_f_.low() ^= static_cast<uint8_t>(flag);
}

bool cpu::test_flag(flag flag) noexcept
{
    const auto f = static_cast<uint8_t>(flag);
    return mask_test(a_f_.low(), f);
}

uint8_t cpu::decode(uint16_t inst, standart_instruction_set_t)
{
    log::info("executing instruction: {:#x}", inst);

    switch(inst) {
        case 0x00: { return nop(); }
        case 0x01: { return load(b_c_, read_immediate(imm16)); }
        case 0x02: { return store(make_address(b_c_), a_f_.high()); }
        case 0x03: { return alu::increment(b_c_); }
        case 0x04: { return alu_.increment(b_c_.high()); }
        case 0x05: { return alu_.decrement(b_c_.high()); }
        case 0x06: { return load(b_c_.high(), read_immediate(imm8)); }
        case 0x07: { return alu_.rotate_left_c_acc(); }
        case 0x08: { return store(make_address(read_immediate(imm16)), stack_pointer_) + 4; }
        case 0x09: { return alu_.add(h_l_, b_c_); }
        case 0x0A: { return load(a_f_.high(), read_data(make_address(b_c_))); }
        case 0x0B: { return alu::decrement(b_c_); }
        case 0x0C: { return alu_.increment(b_c_.low()); }
        case 0x0D: { return alu_.decrement(b_c_.low()); }
        case 0x0E: { return load(b_c_.low(), read_immediate(imm8)); }
        case 0x0F: { return alu_.rotate_right_c_acc(); }
        case 0x10: { return stop(); }
        case 0x11: { return load(d_e_, read_immediate(imm16)); }
        case 0x12: { return store(make_address(d_e_), a_f_.high()); }
        case 0x13: { return alu::increment(d_e_); }
        case 0x14: { return alu_.increment(d_e_.high()); }
        case 0x15: { return alu_.decrement(d_e_.high()); }
        case 0x16: { return load(d_e_.high(), read_immediate(imm8)); }
        case 0x17: { return alu_.rotate_left_acc(); }
        case 0x18: {
            const auto data = read_immediate(imm8);
            return jump_relative(make_address(data));
        }
        case 0x19: { return alu_.add(h_l_, d_e_); }
        case 0x1A: { return load(a_f_.high(), read_data(make_address(d_e_))); }
        case 0x1B: { return alu::decrement(d_e_); }
        case 0x1C: { return alu_.increment(d_e_.low()); }
        case 0x1D: { return alu_.decrement(d_e_.low()); }
        case 0x1E: { return load(d_e_.low(), read_immediate(imm8)); }
        case 0x1F: { return alu_.rotate_right_acc(); }
        case 0x20: {
            const auto data = read_immediate(imm8);
            return jump_relative(!test_flag(flag::zero), make_address(data));
        }
        case 0x21: { return load(h_l_, read_immediate(imm16)); }
        case 0x22: { return store_i(); }
        case 0x23: { return alu::increment(h_l_); }
        case 0x24: { return alu_.increment(h_l_.high()); }
        case 0x25: { return alu_.decrement(h_l_.high()); }
        case 0x26: { return load(h_l_.high(), read_immediate(imm8)); }
        case 0x27: { return alu_.decimal_adjust(); }
        case 0x28: {
            const auto data = read_immediate(imm8);
            return jump_relative(test_flag(flag::zero), make_address(data));
        }
        case 0x29: { return alu_.add(h_l_, h_l_); }
        case 0x2A: { return load_i(); } // 8
        case 0x2B: { return alu::decrement(h_l_); }
        case 0x2C: { return alu_.increment(h_l_.low()); }
        case 0x2D: { return alu_.decrement(h_l_.low()); }
        case 0x2E: { return load(h_l_.low(), read_immediate(imm8)); }
        case 0x2F: { return alu_.complement(); }
        case 0x30: {
            const auto data = read_immediate(imm8);
            return jump_relative(!test_flag(flag::carry), make_address(data));
        }
        case 0x31: { return load(stack_pointer_, read_immediate(imm16)); }
        case 0x32: { return store_d(); }
        case 0x33: { return alu::increment(stack_pointer_); }
        case 0x34: {
            const auto address = make_address(h_l_);
            auto data = read_data(address);
            const auto inc_cycles = alu_.increment(data);
            write_data(address, data);
            return inc_cycles + 8;
        }
        case 0x35: {
            const auto address = make_address(h_l_);
            auto data = read_data(address);
            const auto dec_cycles = alu_.decrement(data);
            write_data(address, data);
            return dec_cycles + 8;
        }
        case 0x36: { return store(make_address(h_l_), read_immediate(imm8)); }
        case 0x37: { /* SCF */
            reset_flag(flag::subtract);
            reset_flag(flag::half_carry);
            set_flag(flag::carry);
            return 4;
        }
        case 0x38: {
            const auto data = read_immediate(imm8);
            return jump_relative(test_flag(flag::carry), make_address(data));
        }
        case 0x39: { return alu_.add(h_l_, stack_pointer_); }
        case 0x3A: { return load_d(); }
        case 0x3B: { return alu::decrement(stack_pointer_); }
        case 0x3C: { return alu_.increment(a_f_.high()); }
        case 0x3D: { return alu_.decrement(a_f_.high()); }
        case 0x3E: { return load(a_f_.high(), read_immediate(imm8)); }
        case 0x3F: { /* CPL */
            reset_flag(flag::subtract);
            reset_flag(flag::half_carry);
            flip_flag(flag::carry);
            return 4;
        }
        case 0x40: { return nop(); } /* LD B,B */
        case 0x41: { return load(b_c_.high(), b_c_.low()); }
        case 0x42: { return load(b_c_.high(), d_e_.high()); }
        case 0x43: { return load(b_c_.high(), d_e_.low()); }
        case 0x44: { return load(b_c_.high(), h_l_.high()); }
        case 0x45: { return load(b_c_.high(), h_l_.low()); }
        case 0x46: { return load(b_c_.high(), read_data(make_address(h_l_))); }
        case 0x47: { return load(b_c_.high(), a_f_.high()); }
        case 0x48: { return load(b_c_.low(), b_c_.high()); }
        case 0x49: { return nop(); } /* LD C,C */
        case 0x4A: { return load(b_c_.low(), d_e_.high()); }
        case 0x4B: { return load(b_c_.low(), d_e_.low()); }
        case 0x4C: { return load(b_c_.low(), h_l_.high()); }
        case 0x4D: { return load(b_c_.low(), h_l_.low()); }
        case 0x4E: { return load(b_c_.low(), read_data(make_address(h_l_))); }
        case 0x4F: { return load(b_c_.low(), a_f_.high()); }
        case 0x50: { return load(d_e_.high(), b_c_.high()); }
        case 0x51: { return load(d_e_.high(), b_c_.low()); }
        case 0x52: { return nop(); } /* LD D,D */
        case 0x53: { return load(d_e_.high(), d_e_.low()); }
        case 0x54: { return load(d_e_.high(), h_l_.high()); }
        case 0x55: { return load(d_e_.high(), h_l_.low()); }
        case 0x56: { return load(d_e_.high(), read_data(make_address(h_l_))); }
        case 0x57: { return load(d_e_.high(), a_f_.high()); }
        case 0x58: { return load(d_e_.low(), b_c_.high()); }
        case 0x59: { return load(d_e_.low(), b_c_.low()); }
        case 0x5A: { return load(d_e_.low(), d_e_.high()); }
        case 0x5B: { return nop(); }  /* LD E,E */
        case 0x5C: { return load(d_e_.low(), h_l_.high()); }
        case 0x5D: { return load(d_e_.low(), h_l_.low()); }
        case 0x5E: { return load(d_e_.low(), read_data(make_address(h_l_))); }
        case 0x5F: { return load(d_e_.low(), a_f_.high()); }
        case 0x60: { return load(h_l_.high(), b_c_.high()); }
        case 0x61: { return load(h_l_.high(), b_c_.low()); }
        case 0x62: { return load(h_l_.high(), d_e_.high()); }
        case 0x63: { return load(h_l_.high(), d_e_.low()); }
        case 0x64: { return nop(); } /* LD H, H */
        case 0x65: { return load(h_l_.high(), h_l_.low()); }
        case 0x66: { return load(h_l_.high(), read_data(make_address(h_l_))); }
        case 0x67: { return load(h_l_.high(), a_f_.high()); }
        case 0x68: { return load(h_l_.low(), b_c_.high()); }
        case 0x69: { return load(h_l_.low(), b_c_.low()); }
        case 0x6A: { return load(h_l_.low(), d_e_.high()); }
        case 0x6B: { return load(h_l_.low(), d_e_.low()); }
        case 0x6C: { return load(h_l_.low(), h_l_.high()); }
        case 0x6D: { return load(h_l_.low(), h_l_.low()); }
        case 0x6E: { return load(h_l_.low(), read_data(make_address(h_l_))); }
        case 0x6F: { return load(h_l_.low(), a_f_.high()); }
        case 0x70: { return store(make_address(h_l_), b_c_.high()); }
        case 0x71: { return store(make_address(h_l_), b_c_.low()); }
        case 0x72: { return store(make_address(h_l_), d_e_.high()); }
        case 0x73: { return store(make_address(h_l_), d_e_.low()); }
        case 0x74: { return store(make_address(h_l_), h_l_.high()); }
        case 0x75: { return store(make_address(h_l_), h_l_.low()); }
        case 0x76: { return halt(); }
        case 0x77: { return store(make_address(h_l_), a_f_.high()); }
        case 0x78: { return load(a_f_.high(), b_c_.high()); }
        case 0x79: { return load(a_f_.high(), b_c_.low()); }
        case 0x7A: { return load(a_f_.high(), d_e_.high()); }
        case 0x7B: { return load(a_f_.high(), d_e_.low()); }
        case 0x7C: { return load(a_f_.high(), h_l_.high()); }
        case 0x7D: { return load(a_f_.high(), h_l_.low()); }
        case 0x7E: { return load(a_f_.high(), read_data(make_address(h_l_))); }
        case 0x7F: { return nop(); } /* LD A,A */
        case 0x80: { return alu_.add(b_c_.high()); }
        case 0x81: { return alu_.add(b_c_.low()); }
        case 0x82: { return alu_.add(d_e_.high()); }
        case 0x83: { return alu_.add(d_e_.low()); }
        case 0x84: { return alu_.add(h_l_.high()); }
        case 0x85: { return alu_.add(h_l_.low()); }
        case 0x86: {
            const auto data = read_data(make_address(h_l_));
            return alu_.add(data) + 4;
        }
        case 0x87: { return alu_.add(a_f_.high()); }
        case 0x88: { return alu_.add_c(b_c_.high()); }
        case 0x89: { return alu_.add_c(b_c_.low()); }
        case 0x8A: { return alu_.add_c(d_e_.high()); }
        case 0x8B: { return alu_.add_c(d_e_.low()); }
        case 0x8C: { return alu_.add_c(h_l_.high()); }
        case 0x8D: { return alu_.add_c(h_l_.low()); }
        case 0x8E: {
            const auto data = read_data(make_address(h_l_));
            return alu_.add_c(data) + 4;
        }
        case 0x8F: { return alu_.add_c(a_f_.high()); }
        case 0x90: { return alu_.subtract(b_c_.high()); }
        case 0x91: { return alu_.subtract(b_c_.low()); }
        case 0x92: { return alu_.subtract(d_e_.high()); }
        case 0x93: { return alu_.subtract(d_e_.low()); }
        case 0x94: { return alu_.subtract(h_l_.high()); }
        case 0x95: { return alu_.subtract(h_l_.low()); }
        case 0x96: {
            const auto data = read_data(make_address(h_l_));
            return alu_.subtract(data) + 4;
        }
        case 0x97: { return alu_.subtract(a_f_.high()); }
        case 0x98: { return alu_.subtract_c(b_c_.high()); }
        case 0x99: { return alu_.subtract_c(b_c_.low()); }
        case 0x9A: { return alu_.subtract_c(d_e_.high()); }
        case 0x9B: { return alu_.subtract_c(d_e_.low()); }
        case 0x9C: { return alu_.subtract_c(h_l_.high()); }
        case 0x9D: { return alu_.subtract_c(h_l_.low()); }
        case 0x9E: {
            const auto data = read_data(make_address(h_l_));
            return alu_.subtract_c(data) + 4;
        }
        case 0x9F: { return alu_.subtract_c(a_f_.high()); }
        case 0xA0: { return alu_.logical_and(b_c_.high()); }
        case 0xA1: { return alu_.logical_and(b_c_.low()); }
        case 0xA2: { return alu_.logical_and(d_e_.high()); }
        case 0xA3: { return alu_.logical_and(d_e_.low()); }
        case 0xA4: { return alu_.logical_and(h_l_.high()); }
        case 0xA5: { return alu_.logical_and(h_l_.low()); }
        case 0xA6: {
            const auto data = read_data(make_address(h_l_));
            return alu_.logical_and(data) + 4;
        }
        case 0xA7: { return nop(); } /* AND A */
        case 0xA8: { return alu_.logical_xor(b_c_.high()); }
        case 0xA9: { return alu_.logical_xor(b_c_.low()); }
        case 0xAA: { return alu_.logical_xor(d_e_.high()); }
        case 0xAB: { return alu_.logical_xor(d_e_.low()); }
        case 0xAC: { return alu_.logical_xor(h_l_.high()); }
        case 0xAD: { return alu_.logical_xor(h_l_.low()); }
        case 0xAE: {
            const auto data = read_data(make_address(h_l_));
            return alu_.logical_xor(data) + 4;
        }
        case 0xAF: { return alu_.logical_xor(a_f_.high()); }
        case 0xB0: { return alu_.logical_or(b_c_.high()); }
        case 0xB1: { return alu_.logical_or(b_c_.low()); }
        case 0xB2: { return alu_.logical_or(d_e_.high()); }
        case 0xB3: { return alu_.logical_or(d_e_.low()); }
        case 0xB4: { return alu_.logical_or(h_l_.high()); }
        case 0xB5: { return alu_.logical_or(h_l_.low()); }
        case 0xB6: {
            const auto data = read_data(make_address(h_l_));
            return alu_.logical_or(data) + 4;
        }
        case 0xB7: { return alu_.logical_or(a_f_.high()); }
        case 0xB8: { return alu_.logical_compare(b_c_.high()); }
        case 0xB9: { return alu_.logical_compare(b_c_.low()); }
        case 0xBA: { return alu_.logical_compare(d_e_.high()); }
        case 0xBB: { return alu_.logical_compare(d_e_.low()); }
        case 0xBC: { return alu_.logical_compare(h_l_.high()); }
        case 0xBD: { return alu_.logical_compare(h_l_.low()); }
        case 0xBE: {
            const auto data = read_data(make_address(h_l_));
            return alu_.logical_compare(data) + 4;
        }
        case 0xBF: { return alu_.logical_compare(a_f_.high()); }
        case 0xC0: { return ret(!test_flag(flag::zero)); }
        case 0xC1: { return pop(b_c_); }
        case 0xC2: {
            const auto data = read_immediate(imm16);
            return jump(!test_flag(flag::zero), make_address(data));
        }
        case 0xC3: {
            const auto data = read_immediate(imm16);
            return jump(make_address(data));
        }
        case 0xC4: {
            const auto data = read_immediate(imm16);
            return call(!test_flag(flag::zero), make_address(data));
        }
        case 0xC5: { return push(b_c_); }
        case 0xC6: {
            const auto data = read_immediate(imm8);
            return alu_.add(data) + 4;
        }
        case 0xC7: { return rst(address8(0x00)); }
        case 0xC8: { return ret(test_flag(flag::zero)); }
        case 0xC9: { return ret(); }
        case 0xCA: {
            const auto data = read_immediate(imm16);
            return jump(test_flag(flag::zero), make_address(data));
        }
        case 0xCC: {
            const auto data = read_immediate(imm16);
            return call(test_flag(flag::zero), make_address(data));
        }
        case 0xCD: {
            const auto data = read_immediate(imm16);
            return call(make_address(data));
        }
        case 0xCE: {
            const auto data = read_immediate(imm8);
            return alu_.add_c(data) + 4;
        }
        case 0xCF: { return rst(address8(0x08)); }
        case 0xD0: { return ret(!test_flag(flag::carry)); }
        case 0xD1: { return pop(d_e_); }
        case 0xD2: {
            const auto data = read_immediate(imm16);
            return jump(!test_flag(flag::carry), make_address(data));
        }
        case 0xD4: {
            const auto data = read_immediate(imm16);
            return call(!test_flag(flag::carry), make_address(data));
        }
        case 0xD5: { return push(d_e_); }
        case 0xD6: {
            const auto data = read_immediate(imm8);
            return alu_.subtract(data) + 4;
        }
        case 0xD7: { return rst(address8(0x10)); }
        case 0xD8: { return ret(test_flag(flag::carry)); }
        case 0xD9: { return reti(); }
        case 0xDA: {
            const auto data = read_immediate(imm16);
            return jump(test_flag(flag::carry), make_address(data));
        }
        case 0xDC: {
            const auto data = read_immediate(imm16);
            return call(test_flag(flag::carry), make_address(data));
        }
        case 0xDE: {
            const auto data = read_immediate(imm8);
            return alu_.subtract_c(data) + 4;
        }
        case 0xDF: { return rst(address8(0x18)); }
        case 0xE0: {
            const uint16_t address = 0xFF00 + read_immediate(imm8);
            return store(make_address(address), a_f_.high()) + 4;
        }
        case 0xE1: { return pop(h_l_); }
        case 0xE2: {
            const auto address = make_address(b_c_.low() + 0xFF00);
            return store(address, a_f_.high());
        }
        case 0xE5: { return push(h_l_); }
        case 0xE6: {
            const auto data = read_immediate(imm8);
            return alu_.logical_and(data) + 4;
        }
        case 0xE7: { return rst(address8(0x20)); }
        case 0xE8: {
            const auto data = static_cast<int8_t>(read_immediate(imm8));
            return alu_.add_to_stack_pointer(data);
        }
        case 0xE9: { return jump(h_l_); }
        case 0xEA: {
            const auto data = read_immediate(imm16);
            return store(make_address(data), a_f_.high()) + 8;
        }
        case 0xEE: {
            const auto data = read_immediate(imm8);
            return alu_.logical_xor(data) + 4;
        }
        case 0xEF: { return rst(address8(0x28)); }
        case 0xF0: {
            const uint16_t address = 0xFF00 + read_immediate(imm8);
            const auto data = read_data(make_address(address));
            return load(a_f_.high(), data) + 4;
        }
        case 0xF1: { return pop(a_f_); }
        case 0xF2: {
            const auto data = read_data(make_address(b_c_.low() + 0xFF00));
            return load(a_f_.high(), data);
        }
        case 0xF3: {
            interrupt_master_enable_ = false;
            return 4;
        }
        case 0xF5: { return push(a_f_); }
        case 0xF6: {
            const auto data = read_immediate(imm8);
            return alu_.logical_or(data) + 4;
        }
        case 0xF7: { return rst(address8(0x30)); }
        case 0xF8: { return load_hlsp(); }
        case 0xF9: { return load(stack_pointer_, h_l_); }
        case 0xFA: {
            const auto addr = read_immediate(imm16);
            const auto data = read_data(make_address(addr));
            return load(a_f_.high(), data) + 8;
        }
        case 0xFB: {
            interrupt_master_enable_ = true;
            return 4;
        }
        case 0xFE: {
            const auto data = read_immediate(imm8);
            return alu_.logical_compare(data) + 4;
        }
        case 0xFF: { return rst(address8(0x38)); }
        default: {
            log::error("unknown instruction: {:#x}, address: {:#x}", inst, stack_pointer_.value() - 1);
            std::abort();
        }
    }
}

uint8_t cpu::decode(uint16_t inst, extended_instruction_set_t)
{
    log::info("executing instruction: CB {:#x}", inst);

    const auto get_bitop_mask = [&]() -> uint8_t {
        return 0x1u << (inst >> 0x3u & 0x7u);
    };

    const auto alu_do_one_param = [&](uint8_t (alu::*func)(uint8_t&) const) -> uint8_t {
        const auto address = make_address(h_l_);
        auto data = read_data(address);
        const auto cycles = std::invoke(func, alu_, data);
        write_data(address, data);
        return cycles + 8;
    };

    switch(inst) {
        case 0x00: { return alu_.rotate_left_c(b_c_.high()); }
        case 0x01: { return alu_.rotate_left_c(b_c_.low()); }
        case 0x02: { return alu_.rotate_left_c(d_e_.high()); }
        case 0x03: { return alu_.rotate_left_c(d_e_.low()); }
        case 0x04: { return alu_.rotate_left_c(h_l_.high()); }
        case 0x05: { return alu_.rotate_left_c(h_l_.low()); }
        case 0x06: { return alu_do_one_param(&alu::rotate_left_c); }
        case 0x07: { return alu_.rotate_left_c(b_c_.high()); }

        case 0x08: { return alu_.rotate_right_c(b_c_.high()); }
        case 0x09: { return alu_.rotate_right_c(b_c_.low()); }
        case 0x0A: { return alu_.rotate_right_c(d_e_.high()); }
        case 0x0B: { return alu_.rotate_right_c(d_e_.low()); }
        case 0x0C: { return alu_.rotate_right_c(h_l_.high()); }
        case 0x0D: { return alu_.rotate_right_c(h_l_.low()); }
        case 0x0E: { return alu_do_one_param(&alu::rotate_right_c); }
        case 0x0F: { return alu_.rotate_right_c(b_c_.high()); }

        case 0x10: { return alu_.rotate_left(b_c_.high()); }
        case 0x11: { return alu_.rotate_left(b_c_.low()); }
        case 0x12: { return alu_.rotate_left(d_e_.high()); }
        case 0x13: { return alu_.rotate_left(d_e_.low()); }
        case 0x14: { return alu_.rotate_left(h_l_.high()); }
        case 0x15: { return alu_.rotate_left(h_l_.low()); }
        case 0x16: { return alu_do_one_param(&alu::rotate_left); }
        case 0x17: { return alu_.rotate_left(b_c_.high()); }

        case 0x18: { return alu_.rotate_right(b_c_.high()); }
        case 0x19: { return alu_.rotate_right(b_c_.low()); }
        case 0x1A: { return alu_.rotate_right(d_e_.high()); }
        case 0x1B: { return alu_.rotate_right(d_e_.low()); }
        case 0x1C: { return alu_.rotate_right(h_l_.high()); }
        case 0x1D: { return alu_.rotate_right(h_l_.low()); }
        case 0x1E: { return alu_do_one_param(&alu::rotate_right); }
        case 0x1F: { return alu_.rotate_right(b_c_.high()); }

        case 0x20: { return alu_.shift_left(b_c_.high()); }
        case 0x21: { return alu_.shift_left(b_c_.low()); }
        case 0x22: { return alu_.shift_left(d_e_.high()); }
        case 0x23: { return alu_.shift_left(d_e_.low()); }
        case 0x24: { return alu_.shift_left(h_l_.high()); }
        case 0x25: { return alu_.shift_left(h_l_.low()); }
        case 0x26: { return alu_do_one_param(&alu::shift_left); }
        case 0x27: { return alu_.shift_left(b_c_.high()); }

        case 0x28: { return alu_.shift_right(b_c_.high(), alu::preserve_last_bit); }
        case 0x29: { return alu_.shift_right(b_c_.low(), alu::preserve_last_bit); }
        case 0x2A: { return alu_.shift_right(d_e_.high(), alu::preserve_last_bit); }
        case 0x2B: { return alu_.shift_right(d_e_.low(), alu::preserve_last_bit); }
        case 0x2C: { return alu_.shift_right(h_l_.high(), alu::preserve_last_bit); }
        case 0x2D: { return alu_.shift_right(h_l_.low(), alu::preserve_last_bit); }
        case 0x2E: {
            const auto address = make_address(h_l_);
            auto data = read_data(address);
            const auto cycles = alu_.shift_right(data, alu::preserve_last_bit);
            write_data(address, data);
            return cycles + 8;
        }
        case 0x2F: { return alu_.shift_right(b_c_.high(), alu::preserve_last_bit); }

        case 0x30: { return alu_.swap(b_c_.high()); }
        case 0x31: { return alu_.swap(b_c_.low()); }
        case 0x32: { return alu_.swap(d_e_.high()); }
        case 0x33: { return alu_.swap(d_e_.low()); }
        case 0x34: { return alu_.swap(h_l_.high()); }
        case 0x35: { return alu_.swap(h_l_.low()); }
        case 0x36: { return alu_do_one_param(&alu::swap); }
        case 0x37: { return alu_.swap(a_f_.high()); }

        case 0x38: { return alu_.shift_right(b_c_.high(), alu::reset_last_bit); }
        case 0x39: { return alu_.shift_right(b_c_.low(), alu::reset_last_bit); }
        case 0x3A: { return alu_.shift_right(d_e_.high(), alu::reset_last_bit); }
        case 0x3B: { return alu_.shift_right(d_e_.low(), alu::reset_last_bit); }
        case 0x3C: { return alu_.shift_right(h_l_.high(), alu::reset_last_bit); }
        case 0x3D: { return alu_.shift_right(h_l_.low(), alu::reset_last_bit); }
        case 0x3E: {
            const auto address = make_address(h_l_);
            auto data = read_data(address);
            const auto cycles = alu_.shift_right(data, alu::reset_last_bit);
            write_data(address, data);
            return cycles + 8;
        }
        case 0x3F: { return alu_.shift_right(a_f_.high(), alu::reset_last_bit); }

        case 0x40:
        case 0x50:
        case 0x60:
        case 0x70:
        case 0x48:
        case 0x58:
        case 0x68:
        case 0x78: {
            return alu_.test(b_c_.high(), get_bitop_mask());
        }

        case 0x41:
        case 0x51:
        case 0x61:
        case 0x71:
        case 0x49:
        case 0x59:
        case 0x69:
        case 0x79: {
            return alu_.test(b_c_.low(), get_bitop_mask());
        }

        case 0x42:
        case 0x52:
        case 0x62:
        case 0x72:
        case 0x4A:
        case 0x5A:
        case 0x6A:
        case 0x7A: {
            return alu_.test(d_e_.high(), get_bitop_mask());
        }

        case 0x43:
        case 0x53:
        case 0x63:
        case 0x73:
        case 0x4B:
        case 0x5B:
        case 0x6B:
        case 0x7B: {
            return alu_.test(d_e_.low(), get_bitop_mask());
        }

        case 0x44:
        case 0x54:
        case 0x64:
        case 0x74:
        case 0x4C:
        case 0x5C:
        case 0x6C:
        case 0x7C: {
            return alu_.test(h_l_.high(), get_bitop_mask());
        }

        case 0x45:
        case 0x55:
        case 0x65:
        case 0x75:
        case 0x4D:
        case 0x5D:
        case 0x6D:
        case 0x7D: {
            return alu_.test(h_l_.low(), get_bitop_mask());
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
            const auto cycles = alu_.test(data, get_bitop_mask());
            return cycles + 8;
        }

        case 0x47:
        case 0x57:
        case 0x67:
        case 0x77:
        case 0x4F:
        case 0x5F:
        case 0x6F:
        case 0x7F: {
            return alu_.test(a_f_.high(), get_bitop_mask());
        }

        case 0x80:
        case 0x90:
        case 0xA0:
        case 0xB0:
        case 0x88:
        case 0x98:
        case 0xA8:
        case 0xB8: {
            return alu_.reset(b_c_.high(), get_bitop_mask());
        }

        case 0x81:
        case 0x91:
        case 0xA1:
        case 0xB1:
        case 0x89:
        case 0x99:
        case 0xA9:
        case 0xB9: {
            return alu_.reset(b_c_.low(), get_bitop_mask());
        }

        case 0x82:
        case 0x92:
        case 0xA2:
        case 0xB2:
        case 0x8A:
        case 0x9A:
        case 0xAA:
        case 0xBA: {
            return alu_.reset(d_e_.high(), get_bitop_mask());
        }

        case 0x83:
        case 0x93:
        case 0xA3:
        case 0xB3:
        case 0x8B:
        case 0x9B:
        case 0xAB:
        case 0xBB: {
            return alu_.reset(d_e_.low(), get_bitop_mask());
        }

        case 0x84:
        case 0x94:
        case 0xA4:
        case 0xB4:
        case 0x8C:
        case 0x9C:
        case 0xAC:
        case 0xBC: {
            return alu_.reset(h_l_.high(), get_bitop_mask());
        }

        case 0x85:
        case 0x95:
        case 0xA5:
        case 0xB5:
        case 0x8D:
        case 0x9D:
        case 0xAD:
        case 0xBD: {
            return alu_.reset(h_l_.low(), get_bitop_mask());
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
            const auto cycles = alu::reset(data, get_bitop_mask());
            write_data(address, data);
            return cycles + 8;
        }

        case 0x87:
        case 0x97:
        case 0xA7:
        case 0xB7:
        case 0x8F:
        case 0x9F:
        case 0xAF:
        case 0xBF: {
            return alu_.reset(a_f_.high(), get_bitop_mask());
        }

        case 0xC0:
        case 0xD0:
        case 0xE0:
        case 0xF0:
        case 0xC8:
        case 0xD8:
        case 0xE8:
        case 0xF8: {
            return alu_.set(b_c_.high(), get_bitop_mask());
        }

        case 0xC1:
        case 0xD1:
        case 0xE1:
        case 0xF1:
        case 0xC9:
        case 0xD9:
        case 0xE9:
        case 0xF9: {
            return alu_.set(b_c_.low(), get_bitop_mask());
        }

        case 0xC2:
        case 0xD2:
        case 0xE2:
        case 0xF2:
        case 0xCA:
        case 0xDA:
        case 0xEA:
        case 0xFA: {
            return alu_.set(d_e_.high(), get_bitop_mask());
        }

        case 0xC3:
        case 0xD3:
        case 0xE3:
        case 0xF3:
        case 0xCB:
        case 0xDB:
        case 0xEB:
        case 0xFB: {
            return alu_.set(d_e_.low(), get_bitop_mask());
        }

        case 0xC4:
        case 0xD4:
        case 0xE4:
        case 0xF4:
        case 0xCC:
        case 0xDC:
        case 0xEC:
        case 0xFC: {
            return alu_.set(h_l_.high(), get_bitop_mask());
        }

        case 0xC5:
        case 0xD5:
        case 0xE5:
        case 0xF5:
        case 0xCD:
        case 0xDD:
        case 0xED:
        case 0xFD: {
            return alu_.set(h_l_.low(), get_bitop_mask());
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
            const auto cycles = alu::set(data, get_bitop_mask());
            write_data(address, data);
            return cycles + 8;
        }

        case 0xC7:
        case 0xD7:
        case 0xE7:
        case 0xF7:
        case 0xCF:
        case 0xDF:
        case 0xEF:
        case 0xFF: {
            return alu_.set(a_f_.high(), get_bitop_mask());
        }

        default: {
            log::error("unknown instruction: {:#x}, address: {:#x}", inst, stack_pointer_.value() - 1);
            std::abort();
        }
    }
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
    log::info("data read: {:#x}", data);
    ++program_counter_;
    return data;
}

uint16_t cpu::read_immediate(imm16_t)
{
    const auto lsb = read_immediate(imm8);
    const auto msb = read_immediate(imm8);

    return word(msb, lsb);
}

uint8_t cpu::nop() noexcept
{
    return 4;
}

uint8_t cpu::halt() noexcept
{
    is_halted_ = true;

    // Check halt bug
    // byte interruptEnabledflag = memory->read(IE_REGISTER);
    // byte interruptflag = memory->read(IF_REGISTER);

    // todo investigate this
    // if(!is_interrupt_master_enabled && (interruptflag & interruptEnabledflag & 0x1F)) {
    //     is_halt_bug_triggered = true;
    // }

    return 0;
}

uint8_t cpu::stop() noexcept
{
    // todo make this a separate instruction to save energy by turning off the system completely
    return halt();
}

uint8_t cpu::push(const register16& reg)
{
    const auto write_to_stack = [&](const register8& reg_8) {
        --stack_pointer_;
        write_data(make_address(stack_pointer_), reg_8.value());
    };

    write_to_stack(reg.high());
    write_to_stack(reg.low());
    return 16;
}

uint8_t cpu::pop(register16& reg)
{
    const auto read_from_stack = [&]() {
        const auto data = read_data(make_address(stack_pointer_));
        ++stack_pointer_;
        return data;
    };

    reg.low() = read_from_stack();
    reg.high() = read_from_stack();
    return 12;
}

uint8_t cpu::rst(const address8& address)
{
    const auto cycles = push(program_counter_);
    program_counter_ = address;
    return cycles;
}

uint8_t cpu::jump(const register16& reg)
{
    program_counter_ = reg;
    return 4;
}

uint8_t cpu::jump(const address16& address)
{
    program_counter_ = address;
    return 16;
}

uint8_t cpu::jump(const bool condition, const address16& address)
{
    if(condition) {
        return jump(address);
    }

    return 12;
}

uint8_t cpu::jump_relative(const address8& address) noexcept
{
    program_counter_ += address;
    return 12;
}

uint8_t cpu::jump_relative(const bool condition, const address8& address) noexcept
{
    if(condition) {
        return jump_relative(address);
    }

    return 8;
}

uint8_t cpu::call(const address16& address)
{
    const auto push_cycles = push(program_counter_);
    program_counter_ = address;
    return push_cycles + 8;
}

uint8_t cpu::call(bool condition, const address16& address)
{
    if(condition) {
        return call(address);
    }

    return 12;
}

uint8_t cpu::reti()
{
    interrupt_master_enable_ = true;
    return ret();
}

uint8_t cpu::ret()
{
    return pop(stack_pointer_) + 4;
}

uint8_t cpu::ret(bool condition)
{
    if(condition) {
        return ret() + 4;
    }

    return 8;
}

uint8_t cpu::store(const address16& address, const uint8_t data)
{
    write_data(address, data);
    return 12;
}

uint8_t cpu::store(const address16& address, const register8& reg)
{
    write_data(address, reg.value());
    return 8;
}

uint8_t cpu::store(const address16& address, const register16& reg)
{
    const auto store_low_cycles = store(address, reg.low());
    const auto store_high_cycles = store(address, reg.high());
    return store_low_cycles + store_high_cycles;
}

uint8_t cpu::load(register8& reg, const uint8_t data) noexcept
{
    reg = data;
    return 8;
}

uint8_t cpu::load(register8& r_left, const register8& r_right) noexcept
{
    r_left = r_right;
    return 4;
}

uint8_t cpu::load(register16& reg, const uint16_t data) noexcept
{
    reg = data;
    return 12;
}

uint8_t cpu::load(register16& r_left, const register16& r_right) noexcept
{
    r_left = r_right;
    return 8;
}

uint8_t cpu::store_i() noexcept
{
    const auto cycles = store(make_address(h_l_), a_f_.high());
    ++h_l_;
    return cycles;
}

uint8_t cpu::store_d() noexcept
{
    const auto cycles = store(make_address(h_l_), a_f_.high());
    --h_l_;
    return cycles;
}

uint8_t cpu::load_i() noexcept
{
    const auto data = read_data(make_address(h_l_));
    const auto cycles = load(a_f_.high(), data);
    ++h_l_;
    return cycles;
}

uint8_t cpu::load_d() noexcept
{
    const auto data = read_data(make_address(h_l_));
    const auto cycles = load(a_f_.high(), data);
    --h_l_;
    return cycles;
}

uint8_t cpu::load_hlsp() noexcept
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

    return load(h_l_, value);
}

} // namespace gameboy
