#ifndef GAMEBOY_DISASSEMBLY_VIEW_H
#define GAMEBOY_DISASSEMBLY_VIEW_H

#include "debugger/disassembly_db.h"
#include "gameboy/memory/address.h"
#include "gameboy/util/observer.h"

namespace gameboy {

class bus;
class cpu_debugger;

class disassembly_view {
public:
    explicit disassembly_view(observer<bus> bus, observer<cpu_debugger> cpu_debugger);

    void draw() noexcept;
    void on_write_access(const address16& addr, uint8_t data) noexcept;

private:
    observer<bus> bus_;
    observer<cpu_debugger> cpu_debugger_;
    instruction::disassembly_db rom_db_;
    instruction::disassembly_db wram_db_;
    instruction::disassembly_db hram_db_;
};

} // namespace gameboy

#endif  // GAMEBOY_DISASSEMBLY_VIEW_H
