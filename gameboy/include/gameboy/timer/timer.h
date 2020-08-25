#ifndef GAMEBOY_TIMER_H
#define GAMEBOY_TIMER_H

#include "gameboy/cpu/register8.h"
#include "gameboy/util/mathutil.h"
#include "gameboy/util/observer.h"

namespace gameboy {

class bus;
class timer_debugger;

class timer {
    friend timer_debugger;

public:
    explicit timer(observer<bus> bus);
    void reset() noexcept;

    void tick(uint8_t cycles);

private:
    observer<bus> bus_;

    uint16_t internal_clock_;
    int8_t tima_reload_cycles_;
    uint8_t timer_clock_overflow_bit_;

    register8 tima_;
    register8 tma_;
    register8 tac_;

    bool enabled_;
    bool previous_tima_reload_bit_;

    void update_internal_clock(uint16_t new_internal_clock) noexcept;
    [[nodiscard]] uint8_t timer_clock_overflow_index_select() const noexcept;

    [[nodiscard]] uint8_t on_read(const address16& address) const noexcept;
    void on_write(const address16& address, uint8_t data) noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_TIMER_H
