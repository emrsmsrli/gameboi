#ifndef GAMEBOY_WAVE_CHANNEL_H
#define GAMEBOY_WAVE_CHANNEL_H

#include <array>

#include "gameboy/apu/data/frequency_data.h"
#include "gameboy/cpu/register8.h"
#include "gameboy/memory/memory_constants.h"

namespace gameboy {

struct wave_channel {
    register8 sound_length;
    register8 output_level;
    frequency_data frequency;

    int32_t length_counter = 0u;
    int32_t timer = 0u;
    uint8_t sample_index = 0u;
    uint8_t output = 0u;

    bool enabled = false;
    bool dac_enabled = false;

    std::array<uint8_t, wave_pattern_range.size()> wave_pattern{};

    void tick() noexcept;
    void length_click() noexcept;
    void restart() noexcept;

    void reset_timer() noexcept { timer = (2048 - frequency.value()) * 2; }
};

} // namespace gameboy

#endif //GAMEBOY_WAVE_CHANNEL_H
