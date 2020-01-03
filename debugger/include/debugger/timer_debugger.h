#ifndef GAMEBOY_TIMER_DEBUGGER_H
#define GAMEBOY_TIMER_DEBUGGER_H

#include "gameboy/util/observer.h"

namespace gameboy {

class timer;

class timer_debugger {
public:
    explicit timer_debugger(observer<timer> timer) noexcept;
   
    void draw() const noexcept;

private:
    observer<timer> timer_;
};

} // namespace gameboy

#endif  //GAMEBOY_TIMER_DEBUGGER_H