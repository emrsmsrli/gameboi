#ifndef GAMEBOY_NOISE_CHANNEL_H
#define GAMEBOY_NOISE_CHANNEL_H

#include "gameboy/apu/data/envelope.h"
#include "gameboy/apu/data/frequency_data.h"
#include "gameboy/apu/data/polynomial_counter.h"

namespace gameboy {

struct noise_channel {
    register8 sound_length;
    envelope envelope;
    polynomial_counter polynomial_counter;
    frequency_control control;

    int32_t timer = 0u;
    uint16_t shift_register = 0u;
    uint8_t output = 0u;
    uint8_t length_counter = 0u;
    uint8_t volume = 0u;

    bool enabled = false;
    bool dac_enabled = false;

    void tick() noexcept;

    void length_click() noexcept;
    void envelope_click() noexcept;
    void shift_register_click() noexcept;

    void restart() noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_NOISE_CHANNEL_H
