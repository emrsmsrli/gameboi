#ifndef GAMEBOY_DEBUGGER_H
#define GAMEBOY_DEBUGGER_H

#include <SFML/Graphics.hpp>
#include "gameboy/util/observer.h"
#include "gameboy/bus.h"
#include "cpu_debugger.h"

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

private:
    observer<bus> bus_;
    cpu_debugger cpu_debugger_;

    sf::Clock delta_clock_;
    sf::RenderWindow window_;
};

} // namespace gameboy

#endif //GAMEBOY_DEBUGGER_H
