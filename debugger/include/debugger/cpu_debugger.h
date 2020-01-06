#ifndef GAMEBOY_CPU_DEBUGGER_H
#define GAMEBOY_CPU_DEBUGGER_H

#include <vector>

#include "gameboy/util/observer.h"
#include "gameboy/memory/address.h"
#include "gameboy/cpu/instruction_info.h"

namespace gameboy {

class cpu;

struct instruction_disassembly {
    uint32_t index = 0u;
    uint32_t bank = 0u;
    ::gameboy::address16 address{0u};
    ::gameboy::instruction::instruction_info info;
    std::string disassembly;

    instruction_disassembly(const uint32_t index,
        const uint32_t bank,
        const ::gameboy::address16 address,
        const ::gameboy::instruction::instruction_info info)
        : index{index},
          bank{bank},
          address{address},
          info{info} {}
};

class cpu_debugger {
public:
    explicit cpu_debugger(observer<cpu> cpu) noexcept;

    void draw() const noexcept;

private:
    observer<cpu> cpu_;
    std::vector<instruction_disassembly> disassemblies_;

    void draw_registers() const noexcept;
    void draw_interrupts() const noexcept;
    void draw_rom_disassembly() const noexcept;
    void draw_rom_disassembly_full() const noexcept;

    void do_draw_rom_disassembly(uint32_t start, uint32_t end) const noexcept;
};

} // namespace gameboy

#endif  //GAMEBOY_CPU_DEBUGGER_H
