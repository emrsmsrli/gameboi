#ifndef GAMEBOY_FREQUENCY_CONTROL_H
#define GAMEBOY_FREQUENCY_CONTROL_H

#include "gameboy/cpu/register8.h"
#include "gameboy/util/mathutil.h"

namespace gameboy::audio {

struct frequency_control {
    register8 reg;

    [[nodiscard]] bool should_restart() const noexcept { return bit::test(reg, 7u); }
    [[nodiscard]] bool use_counter() const noexcept { return bit::test(reg, 6u); }
};

} // namespace gameboy::audio

#endif //GAMEBOY_FREQUENCY_CONTROL_H
