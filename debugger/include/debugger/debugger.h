#ifndef GAMEBOY_DEBUGGER_H
#define GAMEBOY_DEBUGGER_H

#include <SFML/Graphics.hpp>

#include "gameboy/bus.h"
#include "gameboy/util/observer.h"
#include "gameboy/util/delegate.h"
#include "apu_debugger.h"
#include "cpu_debugger.h"
#include "ppu_debugger.h"
#include "timer_debugger.h"
#include "memory_bank_debugger.h"
#include "cartridge_debugger.h"
#include "joypad_debugger.h"

namespace gameboy {

class debugger {
public:
    explicit debugger(observer<bus> bus);
    ~debugger();
    debugger(const debugger&) = delete;
    debugger(debugger&&) = delete;

    debugger& operator=(const debugger&) = delete;
    debugger& operator=(debugger&&) = delete;

    void tick();
    void on_instruction(const address16& addr, const instruction::info& info, uint16_t data) noexcept;

    [[nodiscard]] bool has_execution_breakpoint() { return cpu_debugger_.has_execution_breakpoint(); }
    [[nodiscard]] bool has_read_access_breakpoint(const address16& address)
    {
        return cpu_debugger_.has_access_breakpoint(address, cpu_debugger::access_breakpoint::type::read) ||
            cpu_debugger_.has_access_breakpoint(address, cpu_debugger::access_breakpoint::type::read_write);
    }

    [[nodiscard]] bool has_write_access_breakpoint(const address16& address, const uint8_t data)
    {
        return cpu_debugger_.has_access_breakpoint(address, cpu_debugger::access_breakpoint::type::write, data) ||
            cpu_debugger_.has_access_breakpoint(address, cpu_debugger::access_breakpoint::type::read_write, data) ||
            cpu_debugger_.has_access_breakpoint(address, cpu_debugger::access_breakpoint::type::write) ||
            cpu_debugger_.has_access_breakpoint(address, cpu_debugger::access_breakpoint::type::read_write);
    }

private:
    observer<bus> bus_;
    apu_debugger apu_debugger_;
    cpu_debugger cpu_debugger_;
    cartridge_debugger cartridge_debugger_;
    ppu_debugger ppu_debugger_;
    timer_debugger timer_debugger_;
    joypad_debugger joypad_debugger_;
    memory_bank_debugger memory_bank_debugger_;

    sf::Clock delta_clock_;
    sf::RenderWindow window_;
};

} // namespace gameboy

#endif //GAMEBOY_DEBUGGER_H
