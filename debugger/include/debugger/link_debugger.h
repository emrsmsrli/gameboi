#ifndef GAMEBOY_LINK_DEBUGGER_H
#define GAMEBOY_LINK_DEBUGGER_H

#include "gameboy/util/observer.h"

namespace gameboy {

class link;

class link_debugger {
public:
    explicit link_debugger(const observer<link> link)
        : link_{link} {}

    void draw() noexcept;

private:
    observer<link> link_;
};

} // namespace gameboy

#endif  // GAMEBOY_LINK_DEBUGGER_H
