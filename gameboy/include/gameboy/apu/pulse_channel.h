#ifndef GAMEBOY_PULSE_CHANNEL_H
#define GAMEBOY_PULSE_CHANNEL_H

#include "gameboy/apu/data/sweep.h"
#include "gameboy/apu/data/wave_pattern_duty.h"
#include "gameboy/apu/data/envelope.h"
#include "gameboy/apu/data/frequency.h"

namespace gameboy {

struct pulse_channel {
    bool enabled = false;

    sweep sweep;
    wave_pattern_duty wave_pattern_duty;
    envelope envelope;
    frequency frequency;

    void tick() noexcept;

    void length_click() noexcept;
    void sweep_click() noexcept;
    void env_click() noexcept;

    [[nodiscard]] bool sweep_enabled() const noexcept { return sweep.sweep_time() > 0u && sweep.count() > 0u; }
};

} // namespace gameboy

#endif //GAMEBOY_PULSE_CHANNEL_H
