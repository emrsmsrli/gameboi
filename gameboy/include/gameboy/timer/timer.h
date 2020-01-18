#ifndef GAMEBOY_TIMER_H
#define GAMEBOY_TIMER_H

#include "gameboy/util/observer.h"
#include "gameboy/cpu/register8.h"

namespace gameboy {

class bus;
class timer_debugger;

class timer {
    friend timer_debugger;

public:
    explicit timer(observer<bus> bus);

    void tick(uint8_t cycles);

private:
    observer<bus> bus_;

    uint64_t timer_clock_;
    uint64_t divider_clock_;
    uint64_t base_clock_;

    register8 div_;
    register8 tima_;
    register8 tma_;
    register8 tac_;

    [[nodiscard]] bool timer_enabled() const noexcept;
    [[nodiscard]] std::size_t timer_clock_select() const noexcept;

    [[nodiscard]] uint8_t on_read(const address16& address) const noexcept;
    void on_write(const address16& address, uint8_t data) noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_TIMER_H
