#ifndef GAMEBOY_FREQUENCY_DATA_H
#define GAMEBOY_FREQUENCY_DATA_H

#include "gameboy/apu/data/frequency_control.h"
#include "gameboy/cpu/register8.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

struct frequency_data {
    register8 low;
    frequency_control freq_control;

    [[nodiscard]] bool should_restart() const noexcept { return freq_control.should_restart(); }
    [[nodiscard]] bool use_counter() const noexcept { return freq_control.use_counter(); }
    [[nodiscard]] uint16_t value() const noexcept { return word(freq_control.reg.value() & 0x07u, low.value()); }
};

} // namespace gameboy

#endif //GAMEBOY_FREQUENCY_DATA_H
