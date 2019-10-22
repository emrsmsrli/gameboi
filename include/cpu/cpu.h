#ifndef GAMEBOY_CPU_H
#define GAMEBOY_CPU_H

#include <memory>

#include <cpu/register16.h>
#include <cpu/alu.h>
#include <cpu/interrupt.h>
#include <util/enumutil.h>

namespace gameboy {

class bus;

class cpu {
    friend alu;

public:
    explicit cpu(observer<bus> bus) noexcept;

    [[nodiscard]] uint8_t tick();

    void request_interrupt(interrupt request) noexcept
    {
        interrupt_master_enable_ = true;
        interrupt_flags_ |= request;
    }

private:
    enum class flag : uint8_t {
        none = 0u,
        carry = 0x10u,
        half_carry = 0x20u,
        subtract = 0x40u,
        zero = 0x80u,
        all = carry | half_carry | subtract | zero
    };

    static constexpr struct standart_instruction_set_t {} standard_instruction_set{};
    static constexpr struct extended_instruction_set_t {} extended_instruction_set{};

    static constexpr struct imm8_t {} imm8{};
    static constexpr struct imm16_t {} imm16{};

    observer<bus> bus_;

    alu alu_{make_observer(this)};

    /** accumulator and flags */
    register16 a_f_;

    /** general purpose */
    register16 b_c_;
    register16 d_e_;
    register16 h_l_;

    register16 stack_pointer_;
    register16 program_counter_;

    uint64_t total_cycles_ = 0u;

    interrupt interrupt_flags_{interrupt::none};
    interrupt interrupt_enable_{interrupt::none};
    bool interrupt_master_enable_ = false;

    bool is_interrupt_status_change_pending_ = false;
    bool is_halted_ = false;
    bool is_halt_bug_triggered_ = false;

    void on_ie_write(const address16&, uint8_t data) noexcept;
    [[nodiscard]] uint8_t on_ie_read(const address16&) const noexcept;

    [[nodiscard]] uint8_t decode(uint16_t inst, standart_instruction_set_t);
    [[nodiscard]] uint8_t decode(uint16_t inst, extended_instruction_set_t);

    void set_flag(flag flag) noexcept;
    void reset_flag(flag flag) noexcept;
    void flip_flag(flag flag) noexcept;
    bool test_flag(flag flag) noexcept;

    void write_data(const address16& address, uint8_t data);

    [[nodiscard]] uint8_t read_data(const address16& address) const;
    [[nodiscard]] uint8_t read_immediate(imm8_t);
    [[nodiscard]] uint16_t read_immediate(imm16_t);

    /* instructions */
    [[nodiscard]] static uint8_t nop() noexcept;
    [[nodiscard]] uint8_t halt() noexcept;
    [[nodiscard]] uint8_t stop() noexcept;

    [[nodiscard]] uint8_t push(const register16& reg);
    [[nodiscard]] uint8_t pop(register16& reg);

    uint8_t rst(const address8& address);

    [[nodiscard]] uint8_t jump(const register16& reg);
    [[nodiscard]] uint8_t jump(const address16& address);
    [[nodiscard]] uint8_t jump(bool condition, const address16& address);

    [[nodiscard]] uint8_t jump_relative(const address8& address) noexcept;
    [[nodiscard]] uint8_t jump_relative(bool condition, const address8& address) noexcept;

    [[nodiscard]] uint8_t call(const address16& address);
    [[nodiscard]] uint8_t call(bool condition, const address16& address);

    [[nodiscard]] uint8_t reti();
    [[nodiscard]] uint8_t ret();
    [[nodiscard]] uint8_t ret(bool condition);

    [[nodiscard]] uint8_t store(const address16& address, uint8_t data);
    [[nodiscard]] uint8_t store(const address16& address, const register8& reg);
    [[nodiscard]] uint8_t store(const address16& address, const register16& reg);

    [[nodiscard]] static uint8_t load(register8& reg, uint8_t data) noexcept;
    [[nodiscard]] static uint8_t load(register8& r_left, const register8& r_right) noexcept;

    [[nodiscard]] static uint8_t load(register16& reg, uint16_t data) noexcept;
    [[nodiscard]] static uint8_t load(register16& r_left, const register16& r_right) noexcept;

    [[nodiscard]] uint8_t store_i() noexcept;
    [[nodiscard]] uint8_t store_d() noexcept;

    [[nodiscard]] uint8_t load_i() noexcept;
    [[nodiscard]] uint8_t load_d() noexcept;

    [[nodiscard]] uint8_t load_hlsp() noexcept;
};

DEFINE_ENUM_CLASS_FLAGS(cpu::flag);

}

#endif //GAMEBOY_CPU_H
