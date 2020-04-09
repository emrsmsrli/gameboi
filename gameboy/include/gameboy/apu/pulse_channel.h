#ifndef GAMEBOY_PULSE_CHANNEL_H
#define GAMEBOY_PULSE_CHANNEL_H

#include "gameboy/apu/data/sweep.h"
#include "gameboy/apu/data/wave_data.h"
#include "gameboy/apu/data/envelope.h"
#include "gameboy/apu/data/frequency_data.h"

namespace gameboy {

struct pulse_channel {
    sweep sweep;
    wave_data wave_data;
    envelope envelope;
    frequency_data frequency_data;

    int32_t timer = 0;
    uint8_t length_counter = 0;
    uint8_t volume = 0u;
    uint8_t output = 0u;
    uint8_t waveform_index = 0u;

    bool enabled = false;
    bool dac_enabled = false;

    void tick() noexcept;

    void length_click() noexcept;
    void sweep_click() noexcept;
    void envelope_click() noexcept;

    void restart() noexcept;

    void reset_timer() noexcept { timer = (2048 - frequency_data.value()) * 4; }

    uint16_t sweep_calculation() noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_PULSE_CHANNEL_H
