#ifndef GAMEBOY_CPU_DEBUGGER_H
#define GAMEBOY_CPU_DEBUGGER_H

#include <vector>
#include <string>

#include "gameboy/memory/address.h"
#include "gameboy/util/observer.h"

namespace gameboy {

class cpu;

namespace instruction
{
struct info;
}

class cpu_debugger {
public:
    explicit cpu_debugger(observer<cpu> cpu) noexcept;

    void draw() const noexcept;

private:
    observer<cpu> cpu_;

    std::vector<address16> call_stack_;
    std::vector<std::string> last_executed_instructions_;

    void draw_registers() const noexcept;
    void draw_interrupts() const noexcept;
    void draw_last_100_instructions() const noexcept;
    void draw_call_stack() const noexcept;

    void on_instruction(const address16& addr, const instruction::info& info, uint16_t data) noexcept;
};

} // namespace gameboy

#endif  //GAMEBOY_CPU_DEBUGGER_H
