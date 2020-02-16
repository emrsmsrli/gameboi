#ifndef GAMEBOY_DEBUGGER_H
#define GAMEBOY_DEBUGGER_H

#include <SFML/Graphics.hpp>

#include "gameboy/bus.h"
#include "gameboy/util/observer.h"
#include "gameboy/util/delegate.h"
#include "cpu_debugger.h"
#include "ppu_debugger.h"
#include "timer_debugger.h"
#include "memory_bank_debugger.h"
#include "cartridge_debugger.h"

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
    void on_break(const delegate<void()> delegate) { cartridge_debugger_.on_break(delegate); }
    void check_breakpoints() { cartridge_debugger_.check_breakpoints(); }

private:
    observer<bus> bus_;
    cpu_debugger cpu_debugger_;
    ppu_debugger ppu_debugger_;
    timer_debugger timer_debugger_;
    memory_bank_debugger memory_bank_debugger_;
    cartridge_debugger cartridge_debugger_;

    sf::Clock delta_clock_;
    sf::RenderWindow window_;
    sf::Texture texture_{};
};

} // namespace gameboy

#endif //GAMEBOY_DEBUGGER_H
