#ifndef GAMEBOY_WAVE_DATA_H
#define GAMEBOY_WAVE_DATA_H

#include "gameboy/cpu/register8.h"

namespace gameboy {

struct wave_data {
    register8 reg;

    [[nodiscard]] uint8_t duty() const noexcept { return (reg.value() >> 6u) & 0x03u; }
    [[nodiscard]] uint8_t sound_length() const noexcept { return reg.value() & 0x3Fu; }
};

} // namespace gameboy

#endif //GAMEBOY_WAVE_DATA_H
