#ifndef GAMEBOY_APU_H
#define GAMEBOY_APU_H

#include <cstdint>
#include <array>
#include <vector>

#include "gameboy/apu/pulse_channel.h"
#include "gameboy/apu/wave_channel.h"
#include "gameboy/apu/noise_channel.h"
#include "gameboy/apu/data/control.h"
#include "gameboy/cpu/register8.h"
#include "gameboy/memory/address_range.h"
#include "gameboy/util/observer.h"
#include "gameboy/util/delegate.h"

namespace gameboy {

class bus;
class apu_debugger;

class apu {
    friend apu_debugger;

public:
    static constexpr auto sampling_rate = 44'100u;
    static constexpr auto sample_size = 4096u;

    using sound_buffer = std::vector<int16_t>;
    using sound_buffer_full_func = delegate<void(const sound_buffer&)>;

    explicit apu(observer<bus> bus);

    void tick(uint8_t cycles) noexcept;
    void on_sound_buffer_full(const sound_buffer_full_func on_buffer_full) noexcept { on_buffer_full_ = on_buffer_full; }

private:
    observer<bus> bus_;

    bool power_on_;

    pulse_channel channel_1_;
    pulse_channel channel_2_;
    wave_channel channel_3_;
    noise_channel channel_4_;

    audio::control control_;

    uint16_t frame_sequencer_counter_;
    uint16_t buffer_fill_amount_;
    uint8_t frame_sequencer_;
    uint8_t down_sample_counter_;

#if WITH_DEBUGGER
    std::vector<float> sound_buffer_1_;
    std::vector<float> sound_buffer_2_;
    std::vector<float> sound_buffer_3_;
    std::vector<float> sound_buffer_4_;
#endif //WITH_DEBUGGER

    sound_buffer sound_buffer_;

    sound_buffer_full_func on_buffer_full_;

    void generate_samples() noexcept;

    void on_write(const address16& address, uint8_t data) noexcept;
    [[nodiscard]] uint8_t on_read(const address16& address) const noexcept;

    void on_wave_pattern_write(const address16& address, uint8_t data) noexcept;
    [[nodiscard]] uint8_t on_wave_pattern_read(const address16& address) const noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_APU_H
