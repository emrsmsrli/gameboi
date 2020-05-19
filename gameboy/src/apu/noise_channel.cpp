#include "gameboy/apu/noise_channel.h"

#include <array>

namespace gameboy {

constexpr std::array<uint8_t, 8u> divisor_table{
    8, 16, 32, 48, 64, 80, 96, 112
};

void noise_channel::tick() noexcept
{
    --timer;
    if(timer == 0) {
        timer = divisor_table[polynomial_counter.dividing_ratio()] << polynomial_counter.shift_clock_frequency();
        shift_register_click();

        if(enabled && dac_enabled && !bit::test(lfsr, 0u)) {
            output = volume;
        } else {
            output = 0u;
        }
    }
}

void noise_channel::length_click() noexcept
{
    if(length_counter > 0 && control.use_counter()) {
        --length_counter;
        if(length_counter == 0u) {
            enabled = false;
        }
    }
}

void noise_channel::envelope_click() noexcept
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

void noise_channel::shift_register_click() noexcept
{
    const auto result = bit::extract(lfsr, 0u) ^ bit::extract(lfsr, 1u);
    lfsr >>= 1u;
    lfsr |= result << 14u;

    if(polynomial_counter.has_7_bit_counter_width()) {
        lfsr = (lfsr & 0xBFu) | (result << 6u);
    }
}

void noise_channel::restart() noexcept
{
    enabled = true;
    length_counter = 64u - sound_length;
    timer = divisor_table[polynomial_counter.dividing_ratio()] << polynomial_counter.shift_clock_frequency();

    envelope.timer = envelope.period();
    if(envelope.timer == 0u) {
        envelope.timer = 8;
    }

    volume = envelope.initial_volume();
    lfsr = 0x7FFFu;
}

void noise_channel::disable() noexcept
{
    length_counter = 0u;
    enabled = false;
}

void noise_channel::on_write(const register_index index, const uint8_t data) noexcept
{
    switch(index) {
        case register_index::sound_length:
            sound_length = data & 0x3Fu;
            break;
        case register_index::envelope:
            dac_enabled = (data & 0xF8u) != 0x00u;
            envelope.reg = data;
            break;
        case register_index::polynomial_counter:
            polynomial_counter.reg = data;
            break;
        case register_index::freq_control:
            control.reg = data;
            if(control.should_restart()) {
                restart();
            }
            break;
    }
}

} // namespace gameboy
