#ifndef GAMEBOY_CPU_DEBUGGER_H
#define GAMEBOY_CPU_DEBUGGER_H

#include <optional>
#include <vector>
#include <string>

#include "gameboy/memory/address.h"
#include "gameboy/memory/address_range.h"
#include "gameboy/util/observer.h"

namespace gameboy {

class cpu;
class cartridge_debugger;

namespace instruction
{
struct info;
}

class cpu_debugger {
public:
    struct execution_breakpoint {
        static constexpr auto any_bank = -1;

        address16 address{0u};
        int bank = any_bank;
        bool enabled = true;

        [[nodiscard]] bool operator==(const execution_breakpoint& other) const noexcept
        {
            return address == other.address && bank == other.bank && enabled == other.enabled;
        }
    };

    struct access_breakpoint {
        enum class type {
            read = 0,
            write = 1,
            read_write = 2
        };

        address_range range{0u};
        type access_type{type::read};
        std::optional<uint8_t> data;
        bool enabled = true;

        [[nodiscard]] bool operator==(const access_breakpoint& other) const noexcept
        {
            return range == other.range
                && access_type == other.access_type
                && data == other.data
                && enabled == other.enabled;
        }
    };

    explicit cpu_debugger(observer<cpu> cpu) noexcept;

    void draw() noexcept;
    void on_instruction(const address16& addr, const instruction::info& info, uint16_t data) noexcept;

    [[nodiscard]] bool has_execution_breakpoint() const;
    [[nodiscard]] bool has_execution_breakpoint(const execution_breakpoint& breakpoint) const noexcept;
    [[nodiscard]] bool has_access_breakpoint(
        const address16& address,
        access_breakpoint::type type,
        std::optional<uint8_t> data = std::nullopt,
        bool enabled = true) const noexcept;

    [[nodiscard]] register16 get_pc() const noexcept;

private:
    observer<cpu> cpu_;

    std::vector<execution_breakpoint> execution_breakpoints_;
    std::vector<access_breakpoint> access_breakpoints_;
    std::vector<address16> call_stack_;
    std::vector<std::string> last_executed_instructions_;

    void draw_execution_breakpoints() noexcept;
    void draw_access_breakpoints() noexcept;
    void draw_registers() const noexcept;
    void draw_interrupts() const noexcept;
    void draw_last_100_instructions() const noexcept;
    void draw_call_stack() const noexcept;

    [[nodiscard]] bool has_access_breakpoint(const access_breakpoint& breakpoint) const noexcept;
    [[nodiscard]] bool breakpoint_bank_valid(const address16& addr, int bank) const noexcept;
};

} // namespace gameboy

#endif  //GAMEBOY_CPU_DEBUGGER_H
