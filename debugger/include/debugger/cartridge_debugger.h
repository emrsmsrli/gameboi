#ifndef GAMEBOY_CARTRIDGE_DEBUGGER_H
#define GAMEBOY_CARTRIDGE_DEBUGGER_H

#include <vector>
#include <string>
#include <string_view>

#include "gameboy/memory/address.h"
#include "gameboy/cpu/instruction_info.h"
#include "gameboy/util/observer.h"
#include "gameboy/util/delegate.h"

namespace gameboy {

class cartridge;
class cpu;

struct instruction_disassembly {
    uint32_t bank = 0u;
    address16 address{0u};
    instruction::instruction_info info;
    std::string_view area;
    std::string disassembly;

    instruction_disassembly(
        const uint32_t bank,
        const address16 address,
        const instruction::instruction_info info,
        const std::string_view area)
        : bank{bank},
          address{address},
          info{info},
          area{area} {}
};

class cartridge_debugger {
public:
    explicit cartridge_debugger(observer<cartridge> cartridge, observer<cpu> cpu);

    void draw() noexcept;
    void on_break(const delegate<void()> on_break_delegate) { on_break_ = on_break_delegate; }
    void check_breakpoints();

private:
    observer<cartridge> cartridge_;
    observer<cpu> cpu_;

    std::vector<instruction_disassembly> disassemblies_;
    std::vector<address16> breakpoints_;

    delegate<void()> on_break_;

    [[nodiscard]] bool has_breakpoint(const address16& addr) const noexcept;

    void draw_info() const;
    void draw_rom_disassembly() const noexcept;

    void do_draw_rom_disassembly(const std::vector<instruction_disassembly>& disassemblies, 
        uint32_t start, uint32_t end, bool auto_scroll) const noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_CARTRIDGE_DEBUGGER_H
