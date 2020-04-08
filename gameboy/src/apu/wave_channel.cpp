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
    if(length_counter > 0u && frequency.use_sound_length_counter()) {
        length_counter -= 1u;
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

} // namespace gameboy
