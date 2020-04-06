#ifndef GAMEBOY_WAVE_CHANNEL_H
#define GAMEBOY_WAVE_CHANNEL_H

#include <array>

#include "gameboy/apu/data/frequency.h"
#include "gameboy/cpu/register8.h"
#include "gameboy/memory/memory_constants.h"

namespace gameboy {

struct wave_channel {
    /** Bit 7 - Sound Channel 3 Off  (0=Stop, 1=Playback) */
    bool enabled = false;

    /**
     * Bit 7-0 - Sound length (t1: 0 - 255)
     *
     * Sound Length = (256-t1)*(1/256) seconds
     * This value is used only if Bit 6 in NR34 is set.
     */
    register8 sound_length;

    /**
     *   Bit 6-5 - Select output level (Read/Write)
     *
     * Possible Output levels are:
     *
     *   0: Mute (No sound)
     *   1: 100% Volume (Produce Wave Pattern RAM Data as it is)
     *   2:  50% Volume (Produce Wave Pattern RAM data shifted once to the right)
     *   3:  25% Volume (Produce Wave Pattern RAM data shifted twice to the right)
     */
    register8 output_level;

    frequency frequency;

    /**
     * Wave Pattern RAM
     * Waveform storage for arbitrary sound data
     * This storage area holds 32 4-bit samples that are played back upper 4 bits first.
     */
    std::array<uint8_t, wave_pattern_range.size()> wave_pattern{}; // 0xFF30-0xFF3F

    void tick() noexcept;
    void length_click() noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_WAVE_CHANNEL_H
