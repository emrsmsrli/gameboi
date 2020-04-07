#ifndef GAMEBOY_APU_H
#define GAMEBOY_APU_H

#include <cstdint>
#include <array>

#include "gameboy/apu/pulse_channel.h"
#include "gameboy/apu/wave_channel.h"
#include "gameboy/apu/noise_channel.h"
#include "gameboy/apu/data/channel_control.h"
#include "gameboy/cpu/register8.h"
#include "gameboy/memory/address_range.h"
#include "gameboy/util/observer.h"
#include "gameboy/util/delegate.h"

namespace gameboy {

class bus;

class apu {
public:
    static constexpr auto sample_size = 4096u;

    using sound_buffer = std::vector<int16_t>;
    using sound_buffer_full_func = delegate<void(const sound_buffer&)>;

    explicit apu(observer<bus> bus);

    void tick(uint8_t cycles) noexcept;
    void on_sound_buffer_full(const sound_buffer_full_func on_buffer_full) noexcept { on_buffer_full_ = on_buffer_full; }

private:
    observer<bus> bus_;

    bool enabled_;

    pulse_channel channel_1_;
    pulse_channel channel_2_;
    wave_channel channel_3_;
    noise_channel channel_4_;

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
    channel_control channel_control_;

    uint16_t frame_sequencer_counter_;
    uint16_t buffer_fill_amount_;
    uint8_t frame_sequencer_;
    uint8_t down_sample_counter_;

    sound_buffer sound_buffer_;

    sound_buffer_full_func on_buffer_full_;

    void on_write(const address16& address, uint8_t data) noexcept;
    [[nodiscard]] uint8_t on_read(const address16& address) const noexcept;

    void on_wave_pattern_write(const address16& address, uint8_t data) noexcept;
    [[nodiscard]] uint8_t on_wave_pattern_read(const address16& address) const noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_APU_H
