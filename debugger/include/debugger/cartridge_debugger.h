#ifndef GAMEBOY_CARTRIDGE_DEBUGGER_H
#define GAMEBOY_CARTRIDGE_DEBUGGER_H

#include <vector>

#include "gameboy/util/observer.h"

namespace gameboy {

class cartridge;

class cartridge_debugger {
public:
    explicit cartridge_debugger(observer<cartridge> cartridge);
    void draw() noexcept;

private:
    observer<cartridge> cartridge_;
};

} // namespace gameboy

#endif //GAMEBOY_CARTRIDGE_DEBUGGER_H
