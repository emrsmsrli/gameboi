#ifndef GAMEBOY_NOISE_CHANNEL_H
#define GAMEBOY_NOISE_CHANNEL_H

#include "gameboy/apu/data/envelope.h"
#include "gameboy/apu/data/frequency_data.h"
#include "gameboy/apu/data/polynomial_counter.h"

namespace gameboy {

struct noise_channel {
    enum class register_index {
        sound_length = 1,
        envelope = 2,
        polynomial_counter = 3,
        freq_control = 4
    };

    uint8_t sound_length = 0u;
    audio::envelope envelope;
    audio::polynomial_counter polynomial_counter;
    audio::frequency_control control;

    uint32_t timer = 0u;
    uint16_t lfsr = 0u;
    uint8_t output = 0u;
    uint8_t length_counter = 0u;
    uint8_t volume = 0u;

    bool enabled = false;
    bool dac_enabled = false;

    void on_write(register_index index, uint8_t data) noexcept;

    void tick() noexcept;

    void length_click() noexcept;
    void envelope_click() noexcept;
    void shift_register_click() noexcept;

    void restart() noexcept;
    void disable() noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_NOISE_CHANNEL_H
