#ifndef GAMEBOY_APU_H
#define GAMEBOY_APU_H

#include <cstdint>
#include <array>

#include "gameboy/cpu/register8.h"
#include "gameboy/memory/address_range.h"
#include "gameboy/util/observer.h"

namespace gameboy {

class bus;

class apu {
public:
    static constexpr address_range wave_pattern_range{0xFF30u, 0xFF3Fu};

    explicit apu(observer<bus> bus);

    void tick(uint8_t cycles) noexcept;

private:
    observer<bus> bus_;

    /** channel 1 sweep */
    /**
     * Bit 6-4 - Sweep Time
     * Bit 3   - Sweep Increase/Decrease
     *            0: Addition    (frequency increases)
     *            1: Subtraction (frequency decreases)
     * Bit 2-0 - Number of sweep shift (n: 0-7)
     *
     * Sweep Time:
     * 000: sweep off - no freq change
     * 001: 7.8 ms  (1/128Hz)
     * 010: 15.6 ms (2/128Hz)
     * 011: 23.4 ms (3/128Hz)
     * 100: 31.3 ms (4/128Hz)
     * 101: 39.1 ms (5/128Hz)
     * 110: 46.9 ms (6/128Hz)
     * 111: 54.7 ms (7/128Hz)
     *
     * The change of frequency (NR13,NR14) at each shift is calculated by
     * the following formula where X(0) is initial freq & X(t-1) is last freq:
     *   X(t) = X(t-1) +/- X(t-1)/2^n
     */
    register8 nr_10_;

    /** channel 1 sound length/wave pattern duty */
    /**
     * Bit 7-6 - Wave Pattern Duty (Read/Write)
     * Bit 5-0 - Sound length data (Write Only) (t1: 0-63)
     *
     * Wave Duty:
     *
     *   00: 12.5% ( _-------_-------_------- )
     *   01: 25%   ( __------__------__------ )
     *   10: 50%   ( ____----____----____---- ) (normal)
     *   11: 75%   ( ______--______--______-- )
     *
     * Sound Length = (64-t1)*(1/256) seconds
     * The Length value is used only if Bit 6 in NR14 is set.
     */
    register8 nr_11_;

    /** channel 1 volume envelope */
    /**
     * Bit 7-4 - Initial Volume of envelope (0-0Fh) (0=No Sound)
     * Bit 3   - Envelope Direction (0=Decrease, 1=Increase)
     * Bit 2-0 - Number of envelope sweep (n: 0-7)
     * (If zero, stop envelope operation.)
     *
     * Length of 1 step = n*(1/64) seconds
     */
    register8 nr_12_;

    /** channel 1 frequency lo (8 bits) */
    register8 nr_13_;

    /** channel 1 frequency hi */
    /**
     * Bit 7   - Initial (1=Restart Sound)     (Write Only)
     * Bit 6   - Counter/consecutive selection (Read/Write)
     *           (1=Stop output when length in NR11 expires)
     * Bit 2-0 - Frequency's higher 3 bits (x) (Write Only)
     *
     * Frequency = 131072/(2048-x) Hz
     */
    register8 nr_14_;

    /** channel 2 sound length/wave pattern duty */
    /**
     * Bit 7-6 - Wave Pattern Duty (Read/Write)
     * Bit 5-0 - Sound length data (Write Only) (t1: 0-63)
     *
     * Wave Duty:
     *
     *  00: 12.5% ( _-------_-------_------- )
     *  01: 25%   ( __------__------__------ )
     *  10: 50%   ( ____----____----____---- ) (normal)
     *  11: 75%   ( ______--______--______-- )
     *
     *  Sound Length = (64-t1)*(1/256) seconds
     *  The Length value is used only if Bit 6 in NR24 is set.
     */
    register8 nr_21_;

    /** channel 2 volume envelope */
    /**
     *  Bit 7-4 - Initial Volume of envelope (0-0Fh) (0=No Sound)
     *  Bit 3   - Envelope Direction (0=Decrease, 1=Increase)
     *  Bit 2-0 - Number of envelope sweep (n: 0-7)
     *             (If zero, stop envelope operation.)
     *
     *  Length of 1 step = n*(1/64) seconds
     */
    register8 nr_22_;

    /** channel 2 frequency lo (8 bits) */
    register8 nr_23_;

    /** channel 2 frequency hi */
    /**
     *   Bit 7   - Initial (1=Restart Sound)     (Write Only)
     *   Bit 6   - Counter/consecutive selection (Read/Write)
     *             (1=Stop output when length in NR21 expires)
     *   Bit 2-0 - Frequency's higher 3 bits (x) (Write Only)
     *
     * Frequency = 131072/(2048-x) Hz
     */
    register8 nr_24_;

    /** channel 3 sound on/off */
    /** Bit 7 - Sound Channel 3 Off  (0=Stop, 1=Playback) */
    register8 nr_30_;

    /** channel 3 sound length */
    /**
     * Bit 7-0 - Sound length (t1: 0 - 255)
     *
     * Sound Length = (256-t1)*(1/256) seconds
     * This value is used only if Bit 6 in NR34 is set.
     */
    register8 nr_31_;

    /** channel 3 select output level */
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
    register8 nr_32_;

    /** channel 3 frequency's lower data (8 bits) */
    register8 nr_33_;

    /** channel 3 frequency's higher data */
    /**
     *   Bit 7   - Initial (1=Restart Sound)     (Write Only)
     *   Bit 6   - Counter/consecutive selection (Read/Write)
     *             (1=Stop output when length in NR31 expires)
     *   Bit 2-0 - Frequency's higher 3 bits (x) (Write Only)
     *
     * Frequency = 4194304/(64*(2048-x)) Hz = 65536/(2048-x) Hz
     */
    register8 nr_34_;

    /** channel 4 sound length */
    /**
     * Bit 5-0 - Sound length data
     *
     * Sound Length = (64-t1)*(1/256) seconds
     * The Length value is used only if Bit 6 in NR44 is set.
     */
    register8 nr_41_;

    /** channel 4 volume envelope */
    /**
     *   Bit 7-4 - Initial Volume of envelope (0-0Fh) (0=No Sound)
     *   Bit 3   - Envelope Direction (0=Decrease, 1=Increase)
     *   Bit 2-0 - Number of envelope sweep (n: 0-7)
     *             (If zero, stop envelope operation.)
     *
     * Length of 1 step = n*(1/64) seconds
     */
    register8 nr_42_;

    /** channel 4 polynomial counter */
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
    register8 nr_43_;

    /** channel 4 counter/consecutive; inital */
    /**
     *   Bit 7   - Initial (1=Restart Sound)     (Write Only)
     *   Bit 6   - Counter/consecutive selection (Read/Write)
     *             (1=Stop output when length in NR41 expires)
     */
    register8 nr_44_;

    /** channel control / ON-OFF / volume */
    /**
     * The volume bits specify the "Master Volume" for Left/Right sound output.
     *
     *   Bit 7   - Output Vin to SO2 terminal (1=Enable)
     *   Bit 6-4 - SO2 output level (volume)  (0-7)
     *   Bit 3   - Output Vin to SO1 terminal (1=Enable)
     *   Bit 2-0 - SO1 output level (volume)  (0-7)
     *
     * The Vin signal is received from the game cartridge bus,
     * allowing external hardware in the cartridge to supply a fifth sound channel,
     * additionally to the gameboys internal four channels.
     * As far as I know this feature isn't used by any existing games.
     */
    register8 nr_50_;

    /** selection of sound output terminal */
    /**
     *   Bit 7 - Output sound 4 to SO2 terminal
     *   Bit 6 - Output sound 3 to SO2 terminal
     *   Bit 5 - Output sound 2 to SO2 terminal
     *   Bit 4 - Output sound 1 to SO2 terminal
     *   Bit 3 - Output sound 4 to SO1 terminal
     *   Bit 2 - Output sound 3 to SO1 terminal
     *   Bit 1 - Output sound 2 to SO1 terminal
     *   Bit 0 - Output sound 1 to SO1 terminal
     */
    register8 nr_51_;

    /** sound on/off */
    /**
     * If your GB programs don't use sound then write 00h to this register to save 16% or more on GB power consumption.
     * Disabeling the sound controller by clearing Bit 7 destroys the contents of all sound registers.
     * Also, it is not possible to access any sound registers (execpt FF26) while the sound controller is disabled.
     *
     *   Bit 7 - All sound on/off  (0: stop all sound circuits) (Read/Write)
     *   Bit 3 - Sound 4 ON flag (Read Only)
     *   Bit 2 - Sound 3 ON flag (Read Only)
     *   Bit 1 - Sound 2 ON flag (Read Only)
     *   Bit 0 - Sound 1 ON flag (Read Only)
     *
     * Bits 0-3 of this register are read only status bits, writing to these bits does NOT enable/disable sound.
     * The flags get set when sound output is restarted by setting the Initial flag (Bit 7 in NR14-NR44),
     * the flag remains set until the sound length has expired (if enabled).
     * A volume envelopes which has decreased to zero volume will NOT cause the sound flag to go off
     */
    register8 nr_52_;

    /**
     * Wave Pattern RAM
     * Waveform storage for arbitrary sound data
     * This storage area holds 32 4-bit samples that are played back upper 4 bits first.
     */
    std::array<uint8_t, wave_pattern_range.size()> wave_pattern_; // 0xFF30-0xFF3F

    void on_write(const address16& address, uint8_t data) noexcept;
    [[nodiscard]] uint8_t on_read(const address16& address) const noexcept;

    void on_wave_pattern_write(const address16& address, uint8_t data) noexcept;
    [[nodiscard]] uint8_t on_wave_pattern_read(const address16& address) const noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_APU_H
