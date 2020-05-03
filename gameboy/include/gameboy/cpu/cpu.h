#ifndef GAMEBOY_CPU_H
#define GAMEBOY_CPU_H

#include "gameboy/cpu/alu.h"
#include "gameboy/cpu/interrupt.h"
#include "gameboy/cpu/register16.h"
#include "gameboy/util/enum.h"

#if WITH_DEBUGGER
#include "gameboy/util/delegate.h"
#endif //WITH_DEBUGGER

namespace gameboy {

namespace instruction {
struct info;
} // namespace instruction

class bus;
class cpu_debugger;
class cartridge_debugger;

class cpu {
    friend cpu_debugger;
    friend cartridge_debugger;
    friend alu;

public:
    explicit cpu(observer<bus> bus) noexcept;

    [[nodiscard]] uint8_t tick();

    [[nodiscard]] bool interrupts_enabled() const noexcept { return interrupt_master_enable_; }
    void process_interrupts() noexcept;
    void request_interrupt(interrupt request) noexcept;

    [[nodiscard]] bool is_stopped() const noexcept { return is_stopped_; }
    [[nodiscard]] bool is_in_double_speed() const noexcept;

#if WITH_DEBUGGER
    void on_instruction(
        const delegate<void(const address16&, const instruction::info&, uint16_t)> on_instruction_executed) noexcept
    {
        on_instruction_executed_ = on_instruction_executed;
    }
#endif //WITH_DEBUGGER

private:
    enum class flag : uint8_t {
        none = 0u,
        carry = 0x10u,
        half_carry = 0x20u,
        negative = 0x40u,
        zero = 0x80u,
        all = carry | half_carry | negative | zero
    };

    static constexpr struct standard_instruction_set_t {} standard_instruction_set{};
    static constexpr struct extended_instruction_set_t {} extended_instruction_set{};

    static constexpr struct imm8_t {} imm8{};
    static constexpr struct imm16_t {} imm16{};

    observer<bus> bus_;

    alu alu_;

    /** accumulator and flags */
    register16 a_f_;

    /** general purpose */
    register16 b_c_;
    register16 d_e_;
    register16 h_l_;

    register16 stack_pointer_;
    register16 program_counter_;

    /* speed switch register */
    register8 key_1_;
    
    uint64_t total_cycles_;

    interrupt interrupt_flags_;
    interrupt interrupt_enable_;

    bool interrupt_master_enable_;
    int8_t pending_disable_interrupts_counter_;
    int8_t pending_enable_interrupts_counter_;

    bool is_stopped_;
    bool is_halted_;

    int8_t wait_before_unhalt_cycles_;
    int8_t extra_cycles_;

#if WITH_DEBUGGER
    register16 prev_program_counter_;
    delegate<void(const address16&, const instruction::info&, uint16_t)> on_instruction_executed_;
#endif //WITH_DEBUGGER

    void on_ie_write(const address16&, uint8_t data) noexcept;
    [[nodiscard]] uint8_t on_ie_read(const address16&) const noexcept;

    void on_if_write(const address16&, uint8_t data) noexcept;
    [[nodiscard]] uint8_t on_if_read(const address16&) const noexcept;

    void on_key_1_write(const address16&, uint8_t data) noexcept;
    [[nodiscard]] uint8_t on_key_1_read(const address16&) const noexcept;

    [[nodiscard]] uint8_t decode(uint8_t inst, standard_instruction_set_t);
    [[nodiscard]] uint8_t decode(uint8_t inst, extended_instruction_set_t);

    void set_flag(flag flag) noexcept;
    void reset_flag(flag flag) noexcept;
    void flip_flag(flag flag) noexcept;
    bool test_flag(flag flag) noexcept;

    void write_data(const address16& address, uint8_t data);

    [[nodiscard]] uint8_t read_data(const address16& address) const;
    [[nodiscard]] uint8_t read_immediate(imm8_t);
    [[nodiscard]] uint16_t read_immediate(imm16_t);

    [[nodiscard]] interrupt pending_interrupts() const noexcept;
    [[nodiscard]] bool is_interrupt_requested(interrupt i) const noexcept;

    /* instructions */
    static void nop() noexcept;
    void halt() noexcept;
    void stop() noexcept;

    void push(const register16& reg);
    void pop(register16& reg);

    void rst(const address8& address);

    void jump(const register16& reg);
    void jump(const address16& address);
    void jump_relative(const address8& address) noexcept;
    void call(const address16& address);

    void reti();
    void ret();

    void store(const address16& address, uint8_t data);
    void store(const address16& address, const register8& reg);
    void store(const address16& address, const register16& reg);

    static void load(register8& reg, uint8_t data) noexcept;
    static void load(register8& r_left, const register8& r_right) noexcept;

    static void load(register16& reg, uint16_t data) noexcept;
    static void load(register16& r_left, const register16& r_right) noexcept;

    void load_hlsp(int8_t data) noexcept;

public:
    BITMASKF(flag)
};

} // namespace gameboy

#endif //GAMEBOY_CPU_H
