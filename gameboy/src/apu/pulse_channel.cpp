#include "gameboy/apu/pulse_channel.h"

#include <array>

namespace gameboy {

constexpr std::array<bool, 32u> waveform{
    false, false, false, false, false, false, false, true,
    true, false, false, false, false, false, false, true,
    true, false, false, false, false, true, true, true,
    false, true, true, true, true, true, true, false,
};

void pulse_channel::tick() noexcept
{
    --timer;

    if(timer <= 0) {
        waveform_index = (waveform_index + 1u) & 0x07u;
        reset_timer();
        adjust_waveform_duty_index();
        adjust_output_volume();
    }
}

void pulse_channel::on_write(const register_index index, const uint8_t data)
{
    switch(index) {
        case register_index::sweep:
            sweep.reg = data;
            break;
        case register_index::wave_data:
            wave_data.reg = data;
            adjust_waveform_duty_index();
            adjust_output_volume();
            break;
        case register_index::envelope:
            dac_enabled = (data & 0xF8u) != 0x00u;
            envelope.reg = data;
            envelope.timer = envelope.period();
            volume = envelope.initial_volume();

            adjust_output_volume();
            break;
        case register_index::freq_data:
            frequency_data.low = data;
            break;
        case register_index::freq_control:
            frequency_data.freq_control.reg = data;
            if(frequency_data.should_restart()) {
                restart();
            }
            break;
    }
}

void pulse_channel::length_click() noexcept
{
    if(length_counter > 0 && frequency_data.use_counter()) {
        --length_counter;
        if(length_counter == 0) {
            enabled = false;
            output = 0u;
        }
    }
}

void pulse_channel::sweep_click() noexcept
{
    --sweep.timer;
    if(sweep.timer <= 0) {
        sweep.timer = sweep.period();
        if(sweep.timer == 0) {
            sweep.timer = 8;
        }

        if(sweep.enabled && sweep.period() > 0) {
            if(const auto new_freq = sweep_calculation(); new_freq < 2048 && sweep.shift_count() > 0) {
                sweep.shadow = new_freq;
                frequency_data.low = new_freq & 0x00FFu;
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
        envelope.timer = envelope.period();
        if(envelope.timer == 0) {
            envelope.timer = 8;
        }

        if(envelope.period() > 0) {
            switch(envelope.get_mode()) {
                case audio::envelope::mode::increase:
                    if(volume < 15) {
                        ++volume;
                    }
                    break;
                case audio::envelope::mode::decrease:
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
    envelope.timer = envelope.period();

    sweep.enabled = sweep.period() > 0 || sweep.shift_count() > 0;
    sweep.timer = sweep.period();
    if(sweep.timer == 0) {
        sweep.timer = 8;
    }

    sweep.shadow = frequency_data.value();
    if(sweep.shift_count() > 0) {
        sweep_calculation();
    }

    adjust_output_volume();
}

void pulse_channel::disable() noexcept
{
    length_counter = 0u;
    enabled = false;
    output = 0u;
}

uint16_t pulse_channel::sweep_calculation() noexcept
{
    auto new_freq = sweep.shadow >> sweep.shift_count();
    switch(sweep.get_mode()) {
        case audio::sweep::mode::increase:
            new_freq = sweep.shadow + new_freq;
            break;
        case audio::sweep::mode::decrease:
            new_freq = sweep.shadow - new_freq;
            break;
    }

    if(new_freq > 2047) {
        enabled = false;
        output = 0u;
    }

    return new_freq;
}

void pulse_channel::adjust_output_volume() noexcept
{
    if(enabled && dac_enabled) {
        output = volume;
    } else {
        output = 0u;
    }

    if(!waveform[waveform_duty_index]) {
        output = 0u;
    }
}

} // namespace gameboy
