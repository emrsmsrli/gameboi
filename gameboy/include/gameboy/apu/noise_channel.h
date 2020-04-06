#ifndef GAMEBOY_NOISE_CHANNEL_H
#define GAMEBOY_NOISE_CHANNEL_H

#include "gameboy/apu/data/envelope.h"
#include "gameboy/apu/data/frequency.h"
#include "gameboy/apu/data/polynomial_counter.h"

namespace gameboy {

struct noise_channel {
    bool enabled = false;

    /**
     * Bit 5-0 - Sound length data
     *
     * Sound Length = (64-t1)*(1/256) seconds
     * The Length value is used only if Bit 6 in NR44 is set.
     */
    register8 sound_length;

    /**
     *   Bit 7-4 - Initial Volume of envelope (0-0Fh) (0=No Sound)
     *   Bit 3   - Envelope Direction (0=Decrease, 1=Increase)
     *   Bit 2-0 - Number of envelope sweep (n: 0-7)
     *             (If zero, stop envelope operation.)
     *
     * Length of 1 step = n*(1/64) seconds
     */
    envelope envelope;

    /**
     * The amplitude is randomly switched between high and low at the given frequency.
     * A higher frequency will make the noise to appear 'softer'.
     * When Bit 3 is set, the output will become more regular,
     * and some frequencies will sound more like Tone than Noise.
     *
     *   Bit 7-4 - Shift Clock Frequency (s)
     *   Bit 3   - Counter Step/Width (0=15 bits, 1=7 bits)
     *   Bit 2-0 - Dividing Ratio of Frequencies (r)
     *
     * Frequency = 524288 Hz / r / 2^(s+1) ;For r=0 assume r=0.5 instead
     */
    polynomial_counter polynomial_counter;

    /**
     *   Bit 7   - Initial (1=Restart Sound)     (Write Only)
     *   Bit 6   - Counter/consecutive selection (Read/Write)
     *             (1=Stop output when length in NR41 expires)
     */
    register8 counter; // todo

    void tick() noexcept;

    void length_click() noexcept;
    void env_click() noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_NOISE_CHANNEL_H
