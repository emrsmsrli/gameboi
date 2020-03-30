#ifndef GAMEBOY_TIMER_H
#define GAMEBOY_TIMER_H

#include "gameboy/util/observer.h"
#include "gameboy/cpu/register8.h"
#include "gameboy/util/mathutil.h"

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

    uint64_t internal_clock_;

    register8 tima_;
    register8 tma_;
    register8 tac_;

    [[nodiscard]] bool timer_enabled() const noexcept { return bit_test(tac_, 2u); }
    [[nodiscard]] std::size_t timer_clock_freq_select() const noexcept;

    [[nodiscard]] uint8_t on_read(const address16& address) const noexcept;
    void on_write(const address16& address, uint8_t data) noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_TIMER_H
