#ifndef GAMEBOY_PULSE_CHANNEL_H
#define GAMEBOY_PULSE_CHANNEL_H

#include "gameboy/apu/data/envelope.h"
#include "gameboy/apu/data/frequency_data.h"
#include "gameboy/apu/data/sweep.h"
#include "gameboy/apu/data/wave_data.h"

namespace gameboy {

struct pulse_channel {
    enum class register_index {
        sweep = 0,
        wave_data = 1,
        envelope = 2,
        freq_data = 3,
        freq_control = 4
    };

    audio::sweep sweep;
    audio::wave_data wave_data;
    audio::envelope envelope;
    audio::frequency_data frequency_data;

    size_t waveform_duty_index = 0u;
    int16_t timer = 0;
    uint8_t length_counter = 0;
    uint8_t volume = 0u;
    uint8_t output = 0u;
    uint8_t waveform_index = 0u;

    bool enabled = false;
    bool dac_enabled = false;

    void tick() noexcept;

    void on_write(register_index index, uint8_t data);

    void length_click() noexcept;
    void sweep_click() noexcept;
    void envelope_click() noexcept;

    void restart() noexcept;
    void disable() noexcept;

    void reset_timer() noexcept { timer = (2048 - frequency_data.value()) * 4; }
    void adjust_waveform_duty_index() noexcept { waveform_duty_index = wave_data.duty() * 8 + waveform_index; }

    uint16_t sweep_calculation() noexcept;
    void adjust_output_volume() noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_PULSE_CHANNEL_H
