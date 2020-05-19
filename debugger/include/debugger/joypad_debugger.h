#ifndef GAMEBOY_JOYPAD_DEBUGGER_H
#define GAMEBOY_JOYPAD_DEBUGGER_H

#include "gameboy/util/observer.h"

namespace gameboy {

class joypad;

class joypad_debugger {
public:
    explicit joypad_debugger(const observer<joypad> joypad)
        : joypad_{joypad} {}

    void draw() noexcept;

private:
    observer<joypad> joypad_;
};

} // namespace gameboy

#endif //GAMEBOY_JOYPAD_DEBUGGER_H
