
#include "cpu/CPU.h"
#include "memory/Address.h"
#include "util/Log.h"

void gameboy::cpu::CPU::step()
{
    const auto cycle_count = [&]() {
        if(!is_halted) {
            const auto opcode = read_immediate(tag::Imm8{});
            if(opcode != 0xCB) {
                return decode(opcode, tag::StandardInstructionSet{});
            } else {
                return decode(read_immediate(tag::Imm8{}), tag::ExtendedInstructionSet{});
            }
        } else {
            return static_cast<uint8_t>(0x1);
        }
    }();

    total_cycles += cycle_count;

    // checkPowerMode();
    // checkInterrupts();

    //     gpu_->update((uint8_t)cpu_cycles, interrupt_master_enable_);
    //     apu_->update((uint8_t)cpu_cycles);
    //     link_->update((uint8_t)cpu_cycles);
    //     timer_.update((uint8_t)instr_cycles);
    //
}

void gameboy::cpu::CPU::set_flag(gameboy::cpu::Flag flag)
{
    a_f.get_low() |= static_cast<uint8_t>(flag);
}

void gameboy::cpu::CPU::reset_flag(gameboy::cpu::Flag flag)
{
    a_f.get_low() &= ~static_cast<uint8_t>(flag);
}

void gameboy::cpu::CPU::flip_flag(gameboy::cpu::Flag flag)
{
    a_f.get_low() ^= static_cast<uint8_t>(flag);
}

bool gameboy::cpu::CPU::test_flag(gameboy::cpu::Flag flag)
{
    const auto f = static_cast<uint8_t>(flag);
    return (a_f.get_low() & f) == f;
}

uint8_t gameboy::cpu::CPU::decode(uint16_t inst, gameboy::cpu::tag::StandardInstructionSet)
{
    switch(inst) {
        case 0x00: { return nop(); }
        case 0x01: { return load(b_c, read_immediate(tag::Imm16{})); } // todo 12
        case 0x02: { return store(memory::make_address(b_c), a_f.get_high()); } // todo 8
        case 0x03: { return alu.increment(b_c); }
        case 0x04: { return alu.increment(b_c.get_high()); }
        case 0x05: { return alu.decrement(b_c.get_high()); }
        case 0x06: { return load(b_c.get_high(), read_immediate(tag::Imm8{})); } // todo 8
        case 0x07: { return alu.rotate_left_c_acc(); }
        case 0x08: { return store(memory::make_address(read_immediate(tag::Imm16{})), stack_pointer); } // todo 20
        case 0x09: { return alu.add(h_l, b_c); }
        case 0x0A: { return load(a_f.get_high(), read_data(memory::make_address(b_c))); } // todo 8
        case 0x0B: { return alu.decrement(b_c); }
        case 0x0C: { return alu.increment(b_c.get_low()); }
        case 0x0D: { return alu.decrement(b_c.get_low()); }
        case 0x0E: { return load(b_c.get_low(), read_immediate(tag::Imm8{})); } // todo 8
        case 0x0F: { return alu.rotate_right_c_acc(); }
        case 0x10: { return stop(); }
        case 0x11: { return load(d_e, read_immediate(tag::Imm16{})); } // todo 12
        case 0x12: { return store(memory::make_address(d_e), a_f.get_high()); } // todo 8
        case 0x13: { return alu.increment(d_e); }
        case 0x14: { return alu.increment(d_e.get_high()); }
        case 0x15: { return alu.decrement(d_e.get_high()); }
        case 0x16: { return load(d_e.get_high(), read_immediate(tag::Imm8{})); } // todo 8
        case 0x17: { return alu.rotate_left_acc(); }
        case 0x18: {
            const auto data = read_immediate(tag::Imm8{});
            return jump_relative(memory::make_address(data));
        }
        case 0x19: { return alu.add(h_l, d_e); }
        case 0x1A: { return load(a_f.get_high(), read_data(memory::make_address(d_e))); } // todo 8
        case 0x1B: { return alu.decrement(d_e); }
        case 0x1C: { return alu.increment(d_e.get_low()); }
        case 0x1D: { return alu.decrement(d_e.get_low()); }
        case 0x1E: { return load(d_e.get_low(), read_immediate(tag::Imm8{})); } // todo 8
        case 0x1F: { return alu.rotate_right_acc();}
        case 0x20: {
            const auto data = read_immediate(tag::Imm8{});
            return jump_relative(!test_flag(Flag::zero), memory::make_address(data));
        }
        case 0x21: { return load(h_l, read_immediate(tag::Imm16{})); } // todo 12
        case 0x22: { return store_i(); } // todo 8
        case 0x23: { return alu.increment(h_l); }
        case 0x24: { return alu.increment(h_l.get_high()); }
        case 0x25: { return alu.decrement(h_l.get_high()); }
        case 0x26: { return load(h_l.get_high(), read_immediate(tag::Imm8{})); } // todo 8
        case 0x27: { return alu.decimal_adjust(); }
        case 0x28: {
            const auto data = read_immediate(tag::Imm8{});
            return jump_relative(test_flag(Flag::zero), memory::make_address(data));
        }
        case 0x29: { return alu.add(h_l, h_l); }
        case 0x2A: { return load_i(); } // 8
        case 0x2B: { return alu.decrement(h_l); }
        case 0x2C: { return alu.increment(h_l.get_low()); }
        case 0x2D: { return alu.decrement(h_l.get_low()); }
        case 0x2E: { return load(h_l.get_low(), read_immediate(tag::Imm8{})); }
        case 0x2F: { return alu.complement(); }
        case 0x30: {
            const auto data = read_immediate(tag::Imm8{});
            return jump_relative(!test_flag(Flag::carry), memory::make_address(data));
        }
        case 0x31: { return load(stack_pointer, read_immediate(tag::Imm16{})); } // todo 12
        case 0x32: { return store_d(); } // todo 8
        case 0x33: { return alu.increment(stack_pointer); }
        case 0x34: {
            const auto address = memory::make_address(h_l);
            auto data = read_data(address);
            const auto inc_cycles = alu.increment(data);
            write_data(address, data);
            return inc_cycles + 8;
        }
        case 0x35: {
            const auto address = memory::make_address(h_l);
            auto data = read_data(address);
            const auto dec_cycles = alu.decrement(data);
            write_data(address, data);
            return dec_cycles + 8;
        }
        case 0x36: { return store(memory::make_address(h_l), read_immediate(tag::Imm8{})); } // todo 12
        case 0x37: { /* SCF */
            reset_flag(Flag::subtract);
            reset_flag(Flag::half_carry);
            set_flag(Flag::carry);
            return 4;
        }
        case 0x38: {
            const auto data = read_immediate(tag::Imm8{});
            return jump_relative(test_flag(Flag::carry), memory::make_address(data));
        }
        case 0x39: { return alu.add(h_l, stack_pointer); }
        case 0x3A: { return load_d(); } // todo 8
        case 0x3B: { return alu.decrement(stack_pointer); }
        case 0x3C: { return alu.increment(a_f.get_high()); }
        case 0x3D: { return alu.decrement(a_f.get_high()); }
        case 0x3E: { return load(a_f.get_high(), read_immediate(tag::Imm8{})); } // todo 8
        case 0x3F: { /* CPL */
            reset_flag(Flag::subtract);
            reset_flag(Flag::half_carry);
            flip_flag(Flag::carry);
            return 4;
        }
        case 0x40: { return nop(); } /* LD B,B */
        case 0x41: { return load(b_c.get_high(), b_c.get_low()); }
        case 0x42: { return load(b_c.get_high(), d_e.get_high()); }
        case 0x43: { return load(b_c.get_high(), d_e.get_low()); }
        case 0x44: { return load(b_c.get_high(), h_l.get_high()); }
        case 0x45: { return load(b_c.get_high(), h_l.get_low()); }
        case 0x46: { return load(b_c.get_high(), read_data(memory::make_address(h_l))); } // todo 8
        case 0x47: { return load(b_c.get_high(), a_f.get_high()); }
        case 0x48: { return load(b_c.get_low(), b_c.get_high()); }
        case 0x49: { return nop(); } /* LD C,C */
        case 0x4A: { return load(b_c.get_low(), d_e.get_high()); }
        case 0x4B: { return load(b_c.get_low(), d_e.get_low()); }
        case 0x4C: { return load(b_c.get_low(), h_l.get_high()); }
        case 0x4D: { return load(b_c.get_low(), h_l.get_low()); }
        case 0x4E: { return load(b_c.get_low(), read_data(memory::make_address(h_l))); } // todo 8
        case 0x4F: { return load(b_c.get_low(), a_f.get_high()); }
        case 0x50: { return load(d_e.get_high(), b_c.get_high()); }
        case 0x51: { return load(d_e.get_high(), b_c.get_low()); }
        case 0x52: { return nop(); } /* LD D,D */
        case 0x53: { return load(d_e.get_high(), d_e.get_low()); }
        case 0x54: { return load(d_e.get_high(), h_l.get_high()); }
        case 0x55: { return load(d_e.get_high(), h_l.get_low()); }
        case 0x56: { return load(d_e.get_high(), read_data(memory::make_address(h_l))); } // todo 8
        case 0x57: { return load(d_e.get_high(), a_f.get_high()); }
        case 0x58: { return load(d_e.get_low(), b_c.get_high()); }
        case 0x59: { return load(d_e.get_low(), b_c.get_low()); }
        case 0x5A: { return load(d_e.get_low(), d_e.get_high()); }
        case 0x5B: { return nop(); }  /* LD E,E */
        case 0x5C: { return load(d_e.get_low(), h_l.get_high()); }
        case 0x5D: { return load(d_e.get_low(), h_l.get_low()); }
        case 0x5E: { return load(d_e.get_low(), read_data(memory::make_address(h_l))); } // todo 8
        case 0x5F: { return load(d_e.get_low(), a_f.get_high()); }
        case 0x60: { return load(h_l.get_high(), b_c.get_high()); }
        case 0x61: { return load(h_l.get_high(), b_c.get_low()); }
        case 0x62: { return load(h_l.get_high(), d_e.get_high()); }
        case 0x63: { return load(h_l.get_high(), d_e.get_low()); }
        case 0x64: { return nop(); } /* LD H, H */
        case 0x65: { return load(h_l.get_high(), h_l.get_low()); }
        case 0x66: { return load(h_l.get_high(), read_data(memory::make_address(h_l))); } // todo 8
        case 0x67: { return load(h_l.get_high(), a_f.get_high()); }
        case 0x68: { return load(h_l.get_low(), b_c.get_high()); }
        case 0x69: { return load(h_l.get_low(), b_c.get_low()); }
        case 0x6A: { return load(h_l.get_low(), d_e.get_high()); }
        case 0x6B: { return load(h_l.get_low(), d_e.get_low()); }
        case 0x6C: { return load(h_l.get_low(), h_l.get_high()); }
        case 0x6D: { return load(h_l.get_low(), h_l.get_low()); }
        case 0x6E: { return load(h_l.get_low(), read_data(memory::make_address(h_l))); } // todo 8
        case 0x6F: { return load(h_l.get_low(), a_f.get_high()); }
        case 0x70: { return store(memory::make_address(h_l), b_c.get_high()); } // todo 8
        case 0x71: { return store(memory::make_address(h_l), b_c.get_low()); } // todo 8
        case 0x72: { return store(memory::make_address(h_l), d_e.get_high()); } // todo 8
        case 0x73: { return store(memory::make_address(h_l), d_e.get_low()); } // todo 8
        case 0x74: { return store(memory::make_address(h_l), h_l.get_high()); } // todo 8
        case 0x75: { return store(memory::make_address(h_l), h_l.get_low()); } // todo 8
        case 0x76: { return halt(); }
        case 0x77: { return store(memory::make_address(h_l), a_f.get_high()); } // todo 8
        case 0x78: { return load(a_f.get_high(), b_c.get_high()); }
        case 0x79: { return load(a_f.get_high(), b_c.get_low()); }
        case 0x7A: { return load(a_f.get_high(), d_e.get_high()); }
        case 0x7B: { return load(a_f.get_high(), d_e.get_low()); }
        case 0x7C: { return load(a_f.get_high(), h_l.get_high()); }
        case 0x7D: { return load(a_f.get_high(), h_l.get_low()); }
        case 0x7E: { return load(a_f.get_high(), read_data(memory::make_address(h_l))); } // todo 8
        case 0x7F: { return nop(); } /* LD A,A */
        case 0x80: { return alu.add(b_c.get_high()); }
        case 0x81: { return alu.add(b_c.get_low()); }
        case 0x82: { return alu.add(d_e.get_high()); }
        case 0x83: { return alu.add(d_e.get_low()); }
        case 0x84: { return alu.add(h_l.get_high()); }
        case 0x85: { return alu.add(h_l.get_low()); }
        case 0x86: {
            const auto data = read_data(memory::make_address(h_l));
            return alu.add(data) + 4;
        }
        case 0x87: { return alu.add(a_f.get_high()); }
        case 0x88: { return alu.add_c(b_c.get_high()); }
        case 0x89: { return alu.add_c(b_c.get_low()); }
        case 0x8A: { return alu.add_c(d_e.get_high()); }
        case 0x8B: { return alu.add_c(d_e.get_low()); }
        case 0x8C: { return alu.add_c(h_l.get_high()); }
        case 0x8D: { return alu.add_c(h_l.get_low()); }
        case 0x8E: {
            const auto data = read_data(memory::make_address(h_l));
            return alu.add_c(data) + 4;
        }
        case 0x8F: { return alu.add_c(a_f.get_high()); }
        case 0x90: { return alu.subtract(b_c.get_high()); }
        case 0x91: { return alu.subtract(b_c.get_low()); }
        case 0x92: { return alu.subtract(d_e.get_high()); }
        case 0x93: { return alu.subtract(d_e.get_low()); }
        case 0x94: { return alu.subtract(h_l.get_high()); }
        case 0x95: { return alu.subtract(h_l.get_low()); }
        case 0x96: {
            const auto data = read_data(memory::make_address(h_l));
            return alu.subtract(data) + 4;
        }
        case 0x97: { return alu.subtract(a_f.get_high()); }
        case 0x98: { return alu.subtract_c(b_c.get_high()); }
        case 0x99: { return alu.subtract_c(b_c.get_low()); }
        case 0x9A: { return alu.subtract_c(d_e.get_high()); }
        case 0x9B: { return alu.subtract_c(d_e.get_low()); }
        case 0x9C: { return alu.subtract_c(h_l.get_high()); }
        case 0x9D: { return alu.subtract_c(h_l.get_low()); }
        case 0x9E: {
            const auto data = read_data(memory::make_address(h_l));
            return alu.subtract_c(data) + 4;
        }
        case 0x9F: { return alu.subtract_c(a_f.get_high()); }
        case 0xA0: { return alu.logical_and(b_c.get_high()); }
        case 0xA1: { return alu.logical_and(b_c.get_low()); }
        case 0xA2: { return alu.logical_and(d_e.get_high()); }
        case 0xA3: { return alu.logical_and(d_e.get_low()); }
        case 0xA4: { return alu.logical_and(h_l.get_high()); }
        case 0xA5: { return alu.logical_and(h_l.get_low()); }
        case 0xA6: {
            const auto data = read_data(memory::make_address(h_l));
            return alu.logical_and(data) + 4;
        }
        case 0xA7: { return nop(); } /* AND A */
        case 0xA8: { return alu.logical_xor(b_c.get_high()); }
        case 0xA9: { return alu.logical_xor(b_c.get_low()); }
        case 0xAA: { return alu.logical_xor(d_e.get_high()); }
        case 0xAB: { return alu.logical_xor(d_e.get_low()); }
        case 0xAC: { return alu.logical_xor(h_l.get_high()); }
        case 0xAD: { return alu.logical_xor(h_l.get_low()); }
        case 0xAE: {
            const auto data = read_data(memory::make_address(h_l));
            return alu.logical_xor(data) + 4;
        }
        case 0xAF: { return alu.logical_xor(a_f.get_high()); }
        case 0xB0: { return alu.logical_or(b_c.get_high()); }
        case 0xB1: { return alu.logical_or(b_c.get_low()); }
        case 0xB2: { return alu.logical_or(d_e.get_high()); }
        case 0xB3: { return alu.logical_or(d_e.get_low()); }
        case 0xB4: { return alu.logical_or(h_l.get_high()); }
        case 0xB5: { return alu.logical_or(h_l.get_low()); }
        case 0xB6: {
            const auto data = read_data(memory::make_address(h_l));
            return alu.logical_or(data) + 4;
        }
        case 0xB7: { return alu.logical_or(a_f.get_high()); }
        case 0xB8: { return alu.logical_compare(b_c.get_high()); }
        case 0xB9: { return alu.logical_compare(b_c.get_low()); }
        case 0xBA: { return alu.logical_compare(d_e.get_high()); }
        case 0xBB: { return alu.logical_compare(d_e.get_low()); }
        case 0xBC: { return alu.logical_compare(h_l.get_high()); }
        case 0xBD: { return alu.logical_compare(h_l.get_low()); }
        case 0xBE: {
            const auto data = read_data(memory::make_address(h_l));
            return alu.logical_compare(data) + 4;
        }
        case 0xBF: { return alu.logical_compare(a_f.get_high()); }
        case 0xC0: { return ret(!test_flag(Flag::zero)); }
        case 0xC1: { return pop(b_c); }
        case 0xC2: {
            const auto data = read_immediate(tag::Imm16{});
            return jump(!test_flag(Flag::zero), memory::make_address(data));
        }
        case 0xC3: {
            const auto data = read_immediate(tag::Imm16{});
            return jump(memory::make_address(data));
        }
        case 0xC4: {
            const auto data = read_immediate(tag::Imm16{});
            return call(!test_flag(Flag::zero), memory::make_address(data));
        }
        case 0xC5: { return push(b_c); }
        case 0xC6: {
            const auto data = read_immediate(tag::Imm8{});
            return alu.add(data) + 4;
        }
        case 0xC7: { return rst(memory::Address8(0x00)); }
        case 0xC8: { return ret(test_flag(Flag::zero)); }
        case 0xC9: { return ret(); }
        case 0xCA: {
            const auto data = read_immediate(tag::Imm16{});
            return jump(test_flag(Flag::zero), memory::make_address(data));
        }
        case 0xCC: {
            const auto data = read_immediate(tag::Imm16{});
            return call(test_flag(Flag::zero), memory::make_address(data));
        }
        case 0xCD: {
            const auto data = read_immediate(tag::Imm16{});
            return call(memory::make_address(data));
        }
        case 0xCE: {
            const auto data = read_immediate(tag::Imm8{});
            return alu.add_c(data) + 4;
        }
        case 0xCF: { return rst(memory::Address8(0x08)); }
        case 0xD0: { return ret(!test_flag(Flag::carry)); }
        case 0xD1: { return pop(d_e); }
        case 0xD2: {
            const auto data = read_immediate(tag::Imm16{});
            return jump(!test_flag(Flag::carry), memory::make_address(data));
        }
        case 0xD4: {
            const auto data = read_immediate(tag::Imm16{});
            return call(!test_flag(Flag::carry), memory::make_address(data));
        }
        case 0xD5: { return push(d_e); }
        case 0xD6: {
            const auto data = read_immediate(tag::Imm8{});
            return alu.subtract(data) + 4;
        }
        case 0xD7: { return rst(memory::Address8(0x10)); }
        case 0xD8: { return ret(test_flag(Flag::carry)); }
        case 0xD9: { return reti(); }
        case 0xDA: {
            const auto data = read_immediate(tag::Imm16{});
            return jump(test_flag(Flag::carry), memory::make_address(data));
        }
        case 0xDC: {
            const auto data = read_immediate(tag::Imm16{});
            return call(test_flag(Flag::carry), memory::make_address(data));
        }
        case 0xDE: {
            const auto data = read_immediate(tag::Imm8{});
            return alu.subtract_c(data) + 4;
        }
        case 0xDF: { return rst(memory::Address8(0x18)); }
        case 0xE0: { return 1; } // todo LDH (a8),A
        case 0xE1: { return pop(h_l); }
        case 0xE2: { return 1; } // todo LD (C),A
        case 0xE5: { return push(h_l); }
        case 0xE6: {
            const auto data = read_immediate(tag::Imm8{});
            return alu.logical_and(data) + 4;
        }
        case 0xE7: { return rst(memory::Address8(0x20)); }
        case 0xE8: {
            const auto data = read_immediate(tag::Imm8{});
            return alu.add_to_stack_pointer(data);
        }
        case 0xE9: { return jump(h_l); }
        case 0xEA: { return 1; } // todo LD (a16),A
        case 0xEE: {
            const auto data = read_immediate(tag::Imm8{});
            return alu.logical_xor(data) + 4;
        }
        case 0xEF: { return rst(memory::Address8(0x28)); }
        case 0xF0: { return 1; } // todo LDH A,(a8)
        case 0xF1: { return pop(a_f); }
        case 0xF2: { return 1; } // todo LD A,(C)
        case 0xF3: {
            is_interrupt_master_enabled = false;
            return 4;
        }
        case 0xF5: { return push(a_f); }
        case 0xF6: {
            const auto data = read_immediate(tag::Imm8{});
            return alu.logical_or(data) + 4;
        }
        case 0xF7: { return rst(memory::Address8(0x30)); }
        case 0xF8: { return 1; } // todo LD HL,SP+r8
        case 0xF9: { return 1; } // todo LD SP,HL
        case 0xFA: { return 1; } // todo LD A,(a16)
        case 0xFB: {
            is_interrupt_master_enabled = true;
            return 4;
        }
        case 0xFE: {
            const auto data = read_immediate(tag::Imm8{});
            return alu.logical_compare(data) + 4;
        }
        case 0xFF: { return rst(memory::Address8(0x38)); }
        default:
            log::error("unknown instruction: {0:#x}, address: {0:#x}", inst, stack_pointer.get_value() - 1);
            std::abort();
    }
}

uint8_t gameboy::cpu::CPU::decode(uint16_t inst, gameboy::cpu::tag::ExtendedInstructionSet)
{
    const auto get_bitop_mask = [&]() -> uint8_t {
        return 0x1 << (inst >> 0x3 & 0x7);
    };

    switch(inst) {
        case 0x00: { return alu.rotate_left_c(b_c.get_high()); }
        case 0x01: { return alu.rotate_left_c(b_c.get_low()); }
        case 0x02: { return alu.rotate_left_c(d_e.get_high()); }
        case 0x03: { return alu.rotate_left_c(d_e.get_low()); }
        case 0x04: { return alu.rotate_left_c(h_l.get_high()); }
        case 0x05: { return alu.rotate_left_c(h_l.get_low()); }
        case 0x06: {
            const auto address = memory::make_address(h_l);
            auto data = read_data(address);
            const auto cycles = alu.rotate_left_c(data);
            write_data(address, data);
            return cycles + 8;
        }
        case 0x07: {  return alu.rotate_left_c(b_c.get_high());  }

        case 0x08: { return alu.rotate_right_c(b_c.get_high()); }
        case 0x09: { return alu.rotate_right_c(b_c.get_low()); }
        case 0x0A: { return alu.rotate_right_c(d_e.get_high()); }
        case 0x0B: { return alu.rotate_right_c(d_e.get_low()); }
        case 0x0C: { return alu.rotate_right_c(h_l.get_high()); }
        case 0x0D: { return alu.rotate_right_c(h_l.get_low()); }
        case 0x0E: {
            const auto address = memory::make_address(h_l);
            auto data = read_data(address);
            const auto cycles = alu.rotate_right_c(data);
            write_data(address, data);
            return cycles + 8;
        }
        case 0x0F: { return alu.rotate_right_c(b_c.get_high());    }

        case 0x10: { return alu.rotate_left(b_c.get_high()); }
        case 0x11: { return alu.rotate_left(b_c.get_low()); }
        case 0x12: { return alu.rotate_left(d_e.get_high()); }
        case 0x13: { return alu.rotate_left(d_e.get_low()); }
        case 0x14: { return alu.rotate_left(h_l.get_high()); }
        case 0x15: { return alu.rotate_left(h_l.get_low()); }
        case 0x16: {
            const auto address = memory::make_address(h_l);
            auto data = read_data(address);
            const auto cycles = alu.rotate_left(data);
            write_data(address, data);
            return cycles + 8;
        }
        case 0x17: { return alu.rotate_left(b_c.get_high());  }

        case 0x18: { return alu.rotate_right(b_c.get_high()); }
        case 0x19: { return alu.rotate_right(b_c.get_low()); }
        case 0x1A: { return alu.rotate_right(d_e.get_high()); }
        case 0x1B: { return alu.rotate_right(d_e.get_low()); }
        case 0x1C: { return alu.rotate_right(h_l.get_high()); }
        case 0x1D: { return alu.rotate_right(h_l.get_low()); }
        case 0x1E: {
            const auto address = memory::make_address(h_l);
            auto data = read_data(address);
            const auto cycles = alu.rotate_right(data);
            write_data(address, data);
            return cycles + 8;
        }
        case 0x1F: { return alu.rotate_right(b_c.get_high()); }

        case 0x20: { return alu.shift_left(b_c.get_high()); }
        case 0x21: { return alu.shift_left(b_c.get_low()); }
        case 0x22: { return alu.shift_left(d_e.get_high()); }
        case 0x23: { return alu.shift_left(d_e.get_low()); }
        case 0x24: { return alu.shift_left(h_l.get_high()); }
        case 0x25: { return alu.shift_left(h_l.get_low()); }
        case 0x26: {
            const auto address = memory::make_address(h_l);
            auto data = read_data(address);
            const auto cycles = alu.shift_left(data);
            write_data(address, data);
            return cycles + 8;
        }
        case 0x27: { return alu.shift_left(b_c.get_high()); }

        case 0x28: { return alu.shift_right(b_c.get_high(), tag::PreserveLastBit{}); }
        case 0x29: { return alu.shift_right(b_c.get_low(), tag::PreserveLastBit{}); }
        case 0x2A: { return alu.shift_right(d_e.get_high(), tag::PreserveLastBit{}); }
        case 0x2B: { return alu.shift_right(d_e.get_low(), tag::PreserveLastBit{}); }
        case 0x2C: { return alu.shift_right(h_l.get_high(), tag::PreserveLastBit{}); }
        case 0x2D: { return alu.shift_right(h_l.get_low(), tag::PreserveLastBit{}); }
        case 0x2E: {
            const auto address = memory::make_address(h_l);
            auto data = read_data(address);
            const auto cycles = alu.shift_right(data, tag::PreserveLastBit{});
            write_data(address, data);
            return cycles + 8;
        }
        case 0x2F: { return alu.shift_right(b_c.get_high(), tag::PreserveLastBit{}); }

        case 0x30: { return alu.swap(b_c.get_high()); }
        case 0x31: { return alu.swap(b_c.get_low()); }
        case 0x32: { return alu.swap(d_e.get_high()); }
        case 0x33: { return alu.swap(d_e.get_low()); }
        case 0x34: { return alu.swap(h_l.get_high()); }
        case 0x35: { return alu.swap(h_l.get_low()); }
        case 0x36: {
            const auto address = memory::make_address(h_l);
            auto data = read_data(address);
            const auto cycles = alu.swap(data);
            write_data(address, data);
            return cycles + 8;
        }
        case 0x37: { return alu.swap(a_f.get_high()); }

        case 0x38: { return alu.shift_right(b_c.get_high(), tag::ResetLastBit{}); }
        case 0x39: { return alu.shift_right(b_c.get_low(), tag::ResetLastBit{}); }
        case 0x3A: { return alu.shift_right(d_e.get_high(), tag::ResetLastBit{}); }
        case 0x3B: { return alu.shift_right(d_e.get_low(), tag::ResetLastBit{}); }
        case 0x3C: { return alu.shift_right(h_l.get_high(), tag::ResetLastBit{}); }
        case 0x3D: { return alu.shift_right(h_l.get_low(), tag::ResetLastBit{}); }
        case 0x3E: {
            const auto address = memory::make_address(h_l);
            auto data = read_data(address);
            const auto cycles = alu.shift_right(data, tag::ResetLastBit{});
            write_data(address, data);
            return cycles + 8;
        }
        case 0x3F: { return alu.shift_right(a_f.get_high(), tag::ResetLastBit{}); }

        case 0x40: case 0x50: case 0x60: case 0x70:
        case 0x48: case 0x58: case 0x68: case 0x78: {
            return alu.bit_test(b_c.get_high(), get_bitop_mask());
        }

        case 0x41: case 0x51: case 0x61: case 0x71:
        case 0x49: case 0x59: case 0x69: case 0x79: {
            return alu.bit_test(b_c.get_low(), get_bitop_mask());
        }

        case 0x42: case 0x52: case 0x62: case 0x72:
        case 0x4A: case 0x5A: case 0x6A: case 0x7A: {
            return alu.bit_test(d_e.get_high(), get_bitop_mask());
        }

        case 0x43: case 0x53: case 0x63: case 0x73:
        case 0x4B: case 0x5B: case 0x6B: case 0x7B: {
            return alu.bit_test(d_e.get_low(), get_bitop_mask());
        }

        case 0x44: case 0x54: case 0x64: case 0x74:
        case 0x4C: case 0x5C: case 0x6C: case 0x7C: {
            return alu.bit_test(h_l.get_high(), get_bitop_mask());
        }

        case 0x45: case 0x55: case 0x65: case 0x75:
        case 0x4D: case 0x5D: case 0x6D: case 0x7D: {
            return alu.bit_test(h_l.get_low(), get_bitop_mask());
        }

        case 0x46: case 0x56: case 0x66: case 0x76:
        case 0x4E: case 0x5E: case 0x6E: case 0x7E: {
            const auto data = read_data(memory::make_address(h_l));
            const auto cycles = alu.bit_test(data, get_bitop_mask());
            return cycles + 8;
        }

        case 0x47: case 0x57: case 0x67: case 0x77:
        case 0x4F: case 0x5F: case 0x6F: case 0x7F: {
            return alu.bit_test(a_f.get_high(), get_bitop_mask());
        }

        case 0x80: case 0x90: case 0xA0: case 0xB0:
        case 0x88: case 0x98: case 0xA8: case 0xB8: {
            return alu.bit_reset(b_c.get_high(), get_bitop_mask());
        }

        case 0x81: case 0x91: case 0xA1: case 0xB1:
        case 0x89: case 0x99: case 0xA9: case 0xB9: {
            return alu.bit_reset(b_c.get_low(), get_bitop_mask());
        }

        case 0x82: case 0x92: case 0xA2: case 0xB2:
        case 0x8A: case 0x9A: case 0xAA: case 0xBA: {
            return alu.bit_reset(d_e.get_high(), get_bitop_mask());
        }

        case 0x83: case 0x93: case 0xA3: case 0xB3:
        case 0x8B: case 0x9B: case 0xAB: case 0xBB: {
            return alu.bit_reset(d_e.get_low(), get_bitop_mask());
        }

        case 0x84: case 0x94: case 0xA4: case 0xB4:
        case 0x8C: case 0x9C: case 0xAC: case 0xBC: {
            return alu.bit_reset(h_l.get_high(), get_bitop_mask());
        }

        case 0x85: case 0x95: case 0xA5: case 0xB5:
        case 0x8D: case 0x9D: case 0xAD: case 0xBD: {
            return alu.bit_reset(h_l.get_low(), get_bitop_mask());
        }

        case 0x86: case 0x96: case 0xA6: case 0xB6:
        case 0x8E: case 0x9E: case 0xAE: case 0xBE: {
            const auto address = memory::make_address(h_l);
            auto data = read_data(address);
            const auto cycles = alu.bit_reset(data, get_bitop_mask());
            write_data(address, data);
            return cycles + 8;
        }

        case 0x87: case 0x97: case 0xA7: case 0xB7:
        case 0x8F: case 0x9F: case 0xAF: case 0xBF: {
            return alu.bit_reset(a_f.get_high(), get_bitop_mask());
        }

        case 0xC0: case 0xD0: case 0xE0: case 0xF0:
        case 0xC8: case 0xD8: case 0xE8: case 0xF8: {
            return alu.bit_set(b_c.get_high(), get_bitop_mask());
        }

        case 0xC1: case 0xD1: case 0xE1: case 0xF1:
        case 0xC9: case 0xD9: case 0xE9: case 0xF9: {
            return alu.bit_set(b_c.get_low(), get_bitop_mask());
        }

        case 0xC2: case 0xD2: case 0xE2: case 0xF2:
        case 0xCA: case 0xDA: case 0xEA: case 0xFA: {
            return alu.bit_set(d_e.get_high(), get_bitop_mask());
        }

        case 0xC3: case 0xD3: case 0xE3: case 0xF3:
        case 0xCB: case 0xDB: case 0xEB: case 0xFB: {
            return alu.bit_set(d_e.get_low(), get_bitop_mask());
        }

        case 0xC4: case 0xD4: case 0xE4: case 0xF4:
        case 0xCC: case 0xDC: case 0xEC: case 0xFC: {
            return alu.bit_set(h_l.get_high(), get_bitop_mask());
        }

        case 0xC5: case 0xD5: case 0xE5: case 0xF5:
        case 0xCD: case 0xDD: case 0xED: case 0xFD: {
            return alu.bit_set(h_l.get_low(), get_bitop_mask());
        }

        case 0xC6: case 0xD6: case 0xE6: case 0xF6:
        case 0xCE: case 0xDE: case 0xEE: case 0xFE: {
            const auto address = memory::make_address(h_l);
            auto data = read_data(address);
            const auto cycles =  alu.bit_set(data, get_bitop_mask());
            write_data(address, data);
            return cycles + 8;
        }

        case 0xC7: case 0xD7: case 0xE7: case 0xF7:
        case 0xCF: case 0xDF: case 0xEF: case 0xFF: {
            return alu.bit_set(a_f.get_high(), get_bitop_mask());
        }

        default:
            log::error("unknown instruction: {0:#x}, address: {0:#x}", inst, stack_pointer.get_value() - 1);
            std::abort();
    }
}

void gameboy::cpu::CPU::write_data(const memory::Address16& address, uint8_t data)
{
    memory->write(address, data);
}

void gameboy::cpu::CPU::write_data(const memory::Address16& address, uint16_t data)
{
    memory->write(address, data);
}

uint8_t gameboy::cpu::CPU::read_data(const gameboy::memory::Address16& address)
{
    return memory->read(address);
}

uint8_t gameboy::cpu::CPU::read_immediate(gameboy::cpu::tag::Imm8)
{
    const auto data = read_data(memory::make_address(program_counter));
    ++program_counter;
    return data;
}

uint16_t gameboy::cpu::CPU::read_immediate(gameboy::cpu::tag::Imm16)
{
    const auto lsb = read_immediate(tag::Imm8{});
    const auto msb = read_immediate(tag::Imm8{});

    const uint16_t imm_16 = msb;
    return (imm_16 << 8) | lsb;
}

uint8_t gameboy::cpu::CPU::nop() const
{
    return 4;
}

uint8_t gameboy::cpu::CPU::halt()
{
    is_halted = true;

    // Check halt bug
    // byte interruptEnabledFlag = memory->read(IE_REGISTER);
    // byte interruptFlag = memory->read(IF_REGISTER);

    // todo investigate this
    if(!is_interrupt_master_enabled && (interruptFlag & interruptEnabledFlag & 0x1F)) {
        is_halt_bug_triggered = true;
    }

    return 0;
}

uint8_t gameboy::cpu::CPU::stop()
{
    // todo make this a separate instruction to save energy by turning off the system completely
    return halt();
}

uint8_t gameboy::cpu::CPU::push(const gameboy::cpu::Register16& reg)
{
    --stack_pointer;
    // todo memory->write(memory::Address16(stack_pointer.get_value()), reg.get_high().get_value());
    --stack_pointer;
    // todo memory->write(memory::Address16(stack_pointer.get_value()), reg.get_high().get_value());
    return 16;
}

uint8_t gameboy::cpu::CPU::pop(gameboy::cpu::Register16& reg)
{
    // todo reg.get_low() = memory->read(MemoryAddress{stack_pointer});
    ++stack_pointer;
    // todo reg.get_high() = memory->read(MemoryAddress{stack_pointer});
    ++stack_pointer;
    return 12;
}

uint8_t gameboy::cpu::CPU::rst(const gameboy::memory::Address8& address)
{
    const auto cycles = push(program_counter);
    program_counter = address.get_value();
    return cycles;
}

uint8_t gameboy::cpu::CPU::jump(const gameboy::cpu::Register16& reg)
{
    program_counter = reg.get_value();
    return 4;
}

uint8_t gameboy::cpu::CPU::jump(const gameboy::memory::Address16& address)
{
    program_counter = address.get_value();
    return 16;
}

uint8_t gameboy::cpu::CPU::jump(bool condition, const gameboy::memory::Address16& address)
{
    if(condition) {
        return jump(address);
    }

    return 12;
}

uint8_t gameboy::cpu::CPU::jump_relative(const gameboy::memory::Address8& address)
{
    program_counter = program_counter.get_value() + static_cast<int8_t>(address.get_value());
    return 12;
}

uint8_t gameboy::cpu::CPU::jump_relative(bool condition, const gameboy::memory::Address8& address)
{
    if(condition) {
        return jump_relative(address);
    }

    return 8;
}

uint8_t gameboy::cpu::CPU::call(const gameboy::memory::Address16& address)
{
    const auto push_cycles = push(program_counter);
    program_counter = address;
    return push_cycles + 8;
}

uint8_t gameboy::cpu::CPU::call(bool condition, const gameboy::memory::Address16& address)
{
    if(condition) {
        return call(address);
    }

    return 12;
}

uint8_t gameboy::cpu::CPU::reti()
{
    is_interrupt_master_enabled = true;
    return ret();
}

uint8_t gameboy::cpu::CPU::ret()
{
    return pop(stack_pointer) + 4;
}

uint8_t gameboy::cpu::CPU::ret(bool condition)
{
    if(condition) {
        return ret() + 4;
    }

    return 8;
}
