#ifndef GAMEBOY_CPU_DEBUGGER_H
#define GAMEBOY_CPU_DEBUGGER_H

#include "gameboy/util/observer.h"

namespace gameboy {

class cpu;

class cpu_debugger {
public:
    explicit cpu_debugger(observer<cpu> cpu) noexcept;

    void draw() const noexcept;

private:
    observer<cpu> cpu_;

    void draw_registers() const noexcept;
    void draw_interrupts() const noexcept;
};

} // namespace gameboy

#endif  //GAMEBOY_DEBUGGER_H
