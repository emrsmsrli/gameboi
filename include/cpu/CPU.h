#ifndef GAMEBOY_CPU_H
#define GAMEBOY_CPU_H

#include <memory>
#include <cpu/Register16.h>
#include <cpu/ALU.h>
#include <memory/MMU.h>

namespace gameboy::cpu {
    enum class Flag : uint8_t {
        zero = 0x80u,
        subtract = 0x40u,
        half_carry = 0x20u,
        carry = 0x10u,
        all = 0xF0u
    };

    class CPU {
        friend ALU;

    public:
        explicit CPU(std::shared_ptr<memory::MMU> memory_management_unit)
                : mmu(std::move(memory_management_unit)) { }

        void initialize();
        void tick();

    private:
        static constexpr struct StandardInstructionSet { } standard_instruction_set;
        static constexpr struct ExtendedInstructionSet { } extended_instruction_set;

        static constexpr struct Imm8 { } imm_8;
        static constexpr struct Imm16 { } imm_16;

        std::shared_ptr<memory::MMU> mmu;

        ALU alu{*this};

        /* accumulator and flags */
        Register16 a_f;

        /* general purpose */
        Register16 b_c;
        Register16 d_e;
        Register16 h_l;

        Register16 stack_pointer;
        Register16 program_counter;

        uint64_t total_cycles = 0u;

        bool is_interrupt_master_enabled = false;
        bool is_halted = false;
        bool is_halt_bug_triggered = false;

        [[nodiscard]] uint8_t decode(uint16_t inst, StandardInstructionSet);
        [[nodiscard]] uint8_t decode(uint16_t inst, ExtendedInstructionSet);

        void set_flag(Flag flag);
        void reset_flag(Flag flag);
        void flip_flag(Flag flag);
        bool test_flag(Flag flag);

        void write_data(const memory::Address16& address, uint8_t data);

        uint8_t read_data(const memory::Address16& address);
        uint8_t read_immediate(Imm8);
        uint16_t read_immediate(Imm16);

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

        [[nodiscard]] uint8_t store(const memory::Address16& address, uint8_t data);
        [[nodiscard]] uint8_t store(const memory::Address16& address, const Register8& reg);
        [[nodiscard]] uint8_t store(const memory::Address16& address, const Register16& reg);

        [[nodiscard]] uint8_t load(Register8& reg, uint8_t data) const;
        [[nodiscard]] uint8_t load(Register8& r_left, const Register8& r_right) const;

        [[nodiscard]] uint8_t load(Register16& reg, uint16_t data) const;
        [[nodiscard]] uint8_t load(Register16& r_left, const Register16& r_right) const;

        [[nodiscard]] uint8_t store_i();
        [[nodiscard]] uint8_t store_d();

        [[nodiscard]] uint8_t load_i();
        [[nodiscard]] uint8_t load_d();

        [[nodiscard]] uint8_t load_hlsp();
    };
}

#endif //GAMEBOY_CPU_H
