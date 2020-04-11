#include <array>

#include "gameboy/apu/pulse_channel.h"

namespace gameboy {

constexpr std::array<uint8_t, 32u> waveform{
    0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 1, 1, 1,
    0, 1, 1, 1, 1, 1, 1, 0,
};

void pulse_channel::tick() noexcept
{
    --timer;
    if(timer <= 0) {
        reset_timer();
        waveform_index = (waveform_index + 1u) & 0x07u;
    }

    if(enabled && dac_enabled) {
        output = volume;
    } else {
        output = 0u;
    }

    if(waveform[wave_data.duty() * 8 + waveform_index] == 0u) {
        output = 0u;
    }
}

void pulse_channel::length_click() noexcept
{
    if(length_counter > 0 && frequency_data.use_sound_length_counter()) {
        --length_counter;
        if(length_counter == 0) {
            enabled = false;
        }
    }
}

void pulse_channel::sweep_click() noexcept
{
    --sweep.timer;
    if(sweep.timer <= 0) {
        sweep.timer = sweep.sweep_count();
        if(sweep.enabled && sweep.timer > 0) {
            if(const auto new_freq = sweep_calculation(); new_freq < 2048 && sweep.shift_count() > 0) {
                sweep.shadow = new_freq;
                frequency_data.low = new_freq | 0x00FFu;
                frequency_data.freq_control.reg = (frequency_data.freq_control.reg & 0xF8u) | (new_freq >> 8u);

                sweep_calculation();
            }

            sweep_calculation();
        }
    }
}

void pulse_channel::envelope_click() noexcept
{
    --envelope.timer;
    if(envelope.timer <= 0) {
        envelope.timer = envelope.sweep_count();
        if(envelope.timer > 0) {
            switch(envelope.get_mode()) {
                case envelope::mode::increase:
                    if(volume < 15) {
                        ++volume;
                    }
                    break;
                case envelope::mode::decrease:
                    if(volume > 0) {
                        --volume;
                    }
                    break;
            }
        }
    }
}

void pulse_channel::restart() noexcept
{
    reset_timer();
    enabled = true;
    length_counter = 64u - wave_data.sound_length();

    volume = envelope.initial_volume();
    envelope.timer = envelope.sweep_count();

    sweep.enabled = sweep.sweep_count() > 0 || sweep.shift_count() > 0;
    sweep.timer = sweep.sweep_count();
    sweep.shadow = frequency_data.value();
    if(sweep.shift_count() > 0) {
        sweep_calculation();
    }
}

void pulse_channel::disable() noexcept
{
    length_counter = 0u;
    enabled = false;
}

uint16_t pulse_channel::sweep_calculation() noexcept
{
    auto new_freq = sweep.shadow >> sweep.shift_count();
    switch(sweep.get_mode()) {
        case sweep::mode::increase:
            new_freq = sweep.shadow + new_freq;
            break;
        case sweep::mode::decrease:
            new_freq = sweep.shadow - new_freq;
            break;
    }

    if(new_freq > 2047) {
        enabled = false;
    }

    return new_freq;
}

} // namespace gameboy
