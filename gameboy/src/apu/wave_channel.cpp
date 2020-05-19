#include "gameboy/apu/wave_channel.h"

namespace gameboy {

constexpr std::array shift_table{4u, 0u, 1u, 2u};

void wave_channel::tick() noexcept
{
    --timer;
    if(timer <= 0) {
        reset_timer();
        sample_index = (sample_index + 1u) & 0x1Fu;

        if(enabled && dac_enabled) {
            const auto idx = sample_index / 2;
            output = wave_pattern[idx];

            if(!bit::test(sample_index, 0u)) {
                output >>= 4u;
            }

            output &= 0x0Fu;
            output >>= shift_table[(output_level.value() >> 5u) & 0x3u];
        } else {
            output = 0u;
        }
    }
}

void wave_channel::length_click() noexcept
{
    if(length_counter > 0u && frequency.use_counter()) {
        --length_counter;
        if(length_counter == 0u) {
            enabled = false;
        }
    }
}

void wave_channel::restart() noexcept
{
    enabled = true;
    sample_index = 0u;
    length_counter = sound_length.value();
    reset_timer();
}

void wave_channel::disable() noexcept
{
    length_counter = 0u;
    enabled = false;
}

void wave_channel::on_write(const register_index index, const uint8_t data) noexcept
{
    switch(index) {
        case register_index::enable:
            dac_enabled = bit::test(data, 7u);
            break;
        case register_index::sound_length:
            sound_length = data;
            break;
        case register_index::output_level:
            output_level = data;
            break;
        case register_index::freq_data:
            frequency.low = data;
            break;
        case register_index::freq_control:
            frequency.freq_control.reg = data;
            if(frequency.should_restart()) {
                restart();
            }
            break;
    }
}

} // namespace gameboy
