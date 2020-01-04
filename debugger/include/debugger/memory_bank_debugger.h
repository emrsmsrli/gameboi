#ifndef GAMEBOY_MEMORY_BANK_DEBUGGER_H
#define GAMEBOY_MEMORY_BANK_DEBUGGER_H

#include "gameboy/util/observer.h"
#include "imgui_memory_editor/imgui_memory_editor.h"

namespace gameboy {

class bus;

class memory_bank_debugger {
public:
    explicit memory_bank_debugger(observer<bus> bus) noexcept;

    void draw() noexcept;

private:
    observer<bus> bus_;
    MemoryEditor memory_editor_;
};

} // namespace gameboy

#endif //GAMEBOY_MEMORY_BANK_DEBUGGER_H
