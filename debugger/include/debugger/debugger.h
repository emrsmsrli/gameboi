#ifndef GAMEBOY_DEBUGGER_H
#define GAMEBOY_DEBUGGER_H

#include <SFML/Graphics.hpp>
#include "gameboy/util/observer.h"
#include "gameboy/bus.h"

namespace gameboy {

class debugger {
public:
    explicit debugger(observer<const bus> bus);
    ~debugger();
    debugger(const debugger&) = delete;
    debugger(debugger&&) = delete;

    debugger& operator=(const debugger&) = delete;
    debugger& operator=(debugger&&) = delete;

    void tick();

private:
    observer<const bus> bus_;

    sf::Clock delta_clock_;
    sf::RenderWindow window_;
};

} // namespace gameboy

#endif
