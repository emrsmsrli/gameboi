#ifndef GAMEBOY_CPU_H
#define GAMEBOY_CPU_H

#include "cpu/Register16.h"
#include "cpu/ALU.h"

namespace gameboy::cpu {

    namespace tag {
        struct StandardInstructionSet{};
        struct ExtendedInstructionSet{};

        struct Imm8{};
        struct Imm16{};
    }

    enum class Flag : uint8_t {
        zero = 0x80,
        subtract = 0x40,
        half_carry = 0x20,
        carry = 0x10,
        all = 0xF0
    };

    class CPU {
        friend ALU;

    public:

        // Instruction fetch_next_instruction() const;
        [[nodiscard]] uint8_t decode(uint16_t inst, tag::StandardInstructionSet);
        [[nodiscard]] uint8_t decode(uint16_t inst, tag::ExtendedInstructionSet);

    private:
        ALU alu{*this};

        /* accumulator and flags */
        Register16 a_f;

        /* general purpose */
        Register16 b_c;
        Register16 d_e;
        Register16 h_l;

        Register16 stack_pointer;
        Register16 program_counter;

        uint64_t total_cycles = 0;

        bool is_interrupt_master_enabled = false;
        bool is_halted = false;
        bool is_halt_bug_triggered = false;

        void step();

        void set_flag(Flag flag);
        void reset_flag(Flag flag);
        void flip_flag(Flag flag);
        bool test_flag(Flag flag);

        void write_data(const memory::Address16& address, uint8_t data);
        void write_data(const memory::Address16& address, uint16_t data);

        uint8_t read_data(const memory::Address16& address);
        uint8_t read_immidiate(tag::Imm8);
        uint16_t read_immidiate(tag::Imm16);

        /* instructions */
        [[nodiscard]] uint8_t nop() const;
        [[nodiscard]] uint8_t halt();
        [[nodiscard]] uint8_t stop();

        [[nodiscard]] uint8_t push(const Register16& reg);
        [[nodiscard]] uint8_t pop(Register16& reg);

        [[nodiscard]] uint8_t rst(const memory::Address8& address);

        [[nodiscard]] uint8_t jump(const Register16& reg);
        [[nodiscard]] uint8_t jump(const memory::Address16& address);
        [[nodiscard]] uint8_t jump(bool condition, const memory::Address16& address);

        [[nodiscard]] uint8_t jump_relative(const memory::Address8& address);
        [[nodiscard]] uint8_t jump_relative(bool condition, const memory::Address8& address);

        [[nodiscard]] uint8_t call(const memory::Address16& address);
        [[nodiscard]] uint8_t call(bool condition, const memory::Address16& address);

        [[nodiscard]] uint8_t reti();
        [[nodiscard]] uint8_t ret();
        [[nodiscard]] uint8_t ret(bool condition);

        uint8_t store(const memory::Address16& address, uint8_t data);
        uint8_t store(const memory::Address16& address, const Register8& reg);
        uint8_t store(const memory::Address16& address, const Register16& reg);

        uint8_t load(Register8& reg, uint8_t data);
        uint8_t load(Register8& r_left, const Register8& r_right);

        uint8_t load(Register16& reg, uint16_t data);

        uint8_t store_i();
        uint8_t store_d();

        uint8_t load_i();
        uint8_t load_d();
    };
}

#endif //GAMEBOY_CPU_H
