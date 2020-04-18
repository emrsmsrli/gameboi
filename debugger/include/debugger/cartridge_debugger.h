#ifndef GAMEBOY_CARTRIDGE_DEBUGGER_H
#define GAMEBOY_CARTRIDGE_DEBUGGER_H

#include <vector>

#include "debugger/disassembly.h"
#include "gameboy/memory/address.h"
#include "gameboy/cpu/instruction_info.h"
#include "gameboy/util/observer.h"
#include "gameboy/util/delegate.h"

namespace gameboy {

class cartridge;
class cpu_debugger;

class cartridge_debugger {
public:
    explicit cartridge_debugger(observer<cartridge> cartridge, observer<cpu_debugger> cpu_debugger);

    void draw() noexcept;

private:
    observer<cartridge> cartridge_;
    observer<cpu_debugger> cpu_debugger_;

    std::vector<instruction::disassembly> disassemblies_;

    void draw_info() const;
    void draw_rom_disassembly() const noexcept;

    void do_draw_rom_disassembly(const std::vector<instruction::disassembly>& disassemblies,
        uint32_t start, uint32_t end, bool auto_scroll) const noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_CARTRIDGE_DEBUGGER_H
