#include "gameboy/apu/apu.h"

#include "gameboy/bus.h"
#include "gameboy/memory/mmu.h"

namespace gameboy {

constexpr address16 nr_10_addr{0xFF10u};
constexpr address16 nr_11_addr{0xFF11u};
constexpr address16 nr_12_addr{0xFF12u};
constexpr address16 nr_13_addr{0xFF13u};
constexpr address16 nr_14_addr{0xFF14u};

constexpr address16 nr_21_addr{0xFF16u};
constexpr address16 nr_22_addr{0xFF17u};
constexpr address16 nr_23_addr{0xFF18u};
constexpr address16 nr_24_addr{0xFF19u};

constexpr address16 nr_30_addr{0xFF1Au};
constexpr address16 nr_31_addr{0xFF1Bu};
constexpr address16 nr_32_addr{0xFF1Cu};
constexpr address16 nr_33_addr{0xFF1Du};
constexpr address16 nr_34_addr{0xFF1Eu};

constexpr address16 nr_41_addr{0xFF20u};
constexpr address16 nr_42_addr{0xFF21u};
constexpr address16 nr_43_addr{0xFF22u};
constexpr address16 nr_44_addr{0xFF23u};

constexpr address16 nr_50_addr{0xFF24u};
constexpr address16 nr_51_addr{0xFF25u};
constexpr address16 nr_52_addr{0xFF26u};

constexpr std::array apu_register_addresses{
    nr_10_addr, nr_11_addr, nr_12_addr, nr_13_addr, nr_14_addr,
    nr_21_addr, nr_22_addr, nr_23_addr, nr_24_addr,
    nr_30_addr, nr_31_addr, nr_32_addr, nr_33_addr, nr_34_addr,
    nr_41_addr, nr_42_addr, nr_43_addr, nr_44_addr,
    nr_50_addr, nr_51_addr, nr_52_addr
};

constexpr auto frame_sequence_count = 8192u;
constexpr auto frame_sequencer_max = 8u;
constexpr auto down_sample_count = 4'194'304u / 44100u; // cpu clock speed / sample rate

apu::apu(const observer<bus> bus)
    : bus_{bus},
      power_on_{true},
      channel_1_{
          audio::sweep{register8{0x80u}},
          audio::wave_data{register8{0xBFu}},
          audio::envelope{register8{0xF3u}},
          audio::frequency_data{
              register8{0xFFu},
              audio::frequency_control{register8{0xBFu}}
          },
      },
      channel_2_{
          audio::sweep{register8{0xFFu}}, // no sweep
          audio::wave_data{register8{0x3Fu}},
          audio::envelope{register8{0x00u}},
          audio::frequency_data{
              register8{0xFFu},
              audio::frequency_control{register8{0xBFu}}
          },
      },
      channel_3_{
          register8{0xFFu},
          register8{0x9Fu},
          audio::frequency_data{
              register8{0xFFu},
              audio::frequency_control{register8{0xBFu}}
          },
      },
      channel_4_{
          0xFFu,
          audio::envelope{register8{0x00u}},
          audio::polynomial_counter{register8{0x00u}},
          audio::frequency_control{register8{0xBFu}}
      },
      control_{
          register8{0x77u},
          register8{0xF3u}
      },
      frame_sequencer_counter_{frame_sequence_count},
      buffer_fill_amount_{0u},
      frame_sequencer_{0u},
      down_sample_counter_{down_sample_count},
      sound_buffer_(sample_size)
{
    auto mmu = bus->get_mmu();

    for(const auto& addr : apu_register_addresses) {
        mmu->add_memory_delegate(addr, {
            {connect_arg<&apu::on_read>, this},
            {connect_arg<&apu::on_write>, this},
        });
    }

    for(const auto& addr : wave_pattern_range) {
        mmu->add_memory_delegate(make_address(addr), {
            {connect_arg<&apu::on_wave_pattern_read>, this},
            {connect_arg<&apu::on_wave_pattern_write>, this},
        });
    }

    channel_1_.enabled = true;

#if WITH_DEBUGGER
    sound_buffer_1_.resize(sample_size / 2, 0.f);
    sound_buffer_2_.resize(sample_size / 2, 0.f);
    sound_buffer_3_.resize(sample_size / 2, 0.f);
    sound_buffer_4_.resize(sample_size / 2, 0.f);
#endif //WITH_DEBUGGER
}

void apu::tick(uint8_t cycles) noexcept
{
    const auto length_click = [&]() {
        channel_1_.length_click();
        channel_2_.length_click();
        channel_3_.length_click();
        channel_4_.length_click();
    };

    for(; cycles > 0; --cycles, --frame_sequencer_counter_) {
        if(frame_sequencer_counter_ == 0u) {
            frame_sequencer_counter_ = frame_sequence_count;
            switch(frame_sequencer_) {
                case 0u:
                case 4u:
                    length_click();
                    break;
                case 2u:
                case 6u:
                    channel_1_.sweep_click();
                    length_click();
                    break;
                case 7:
                    channel_1_.envelope_click();
                    channel_2_.envelope_click();
                    channel_4_.envelope_click();
                    break;
                default:
                    break;
            }

            ++frame_sequencer_;
            if(frame_sequencer_ == frame_sequencer_max) {
                frame_sequencer_ = 0u;
            }
        }

        channel_1_.tick();
        channel_2_.tick();
        channel_3_.tick();
        channel_4_.tick();

        --down_sample_counter_;
        if(down_sample_counter_ == 0u) {
            down_sample_counter_ = down_sample_count;

            generate_samples();

            if(buffer_fill_amount_ == sample_size) {
                buffer_fill_amount_ = 0u;
                on_buffer_full_(sound_buffer_);
                break;
            }
        }
    }
}

void apu::generate_samples() noexcept
{
    const std::array channel_outputs{
        static_cast<float>(channel_1_.output) / 15.f,
        static_cast<float>(channel_2_.output) / 15.f,
        static_cast<float>(channel_3_.output) / 15.f,
        static_cast<float>(channel_4_.output) / 15.f,
    };

#if WITH_DEBUGGER
    sound_buffer_1_[buffer_fill_amount_ / 2] = channel_outputs[0];
    sound_buffer_2_[buffer_fill_amount_ / 2] = channel_outputs[1];
    sound_buffer_3_[buffer_fill_amount_ / 2] = channel_outputs[2];
    sound_buffer_4_[buffer_fill_amount_ / 2] = channel_outputs[3];
#endif //WITH_DEBUGGER

    const auto sample_for_terminal = [&](const audio::control::terminal terminal) {
        constexpr auto amplitude = 30000.f;
        float sample = 0.f;

        for(auto channel_no = 0; channel_no < channel_outputs.size(); ++channel_no) {
            if(control_.channel_enabled_on_terminal(channel_no, terminal)) {
                sample += channel_outputs[channel_no];
            }
        }

        sample /= channel_outputs.size();

        const auto terminal_volume = control_.terminal_volume<float>(terminal) / 7.f;
        sound_buffer_[buffer_fill_amount_++] = sample * terminal_volume * amplitude;
    };

    sample_for_terminal(audio::control::terminal::left);
    sample_for_terminal(audio::control::terminal::right);
}

void apu::on_write(const address16& address, const uint8_t data) noexcept
{
    if(!power_on_ && address != nr_52_addr) {
        return;
    }

    static constexpr address_range channel_1_range{nr_10_addr, nr_14_addr};
    static constexpr address_range channel_2_range{nr_21_addr, nr_24_addr};
    static constexpr address_range channel_3_range{nr_30_addr, nr_34_addr};
    static constexpr address_range channel_4_range{nr_41_addr, nr_44_addr};

    if(const auto idx = (address.value() - 1) % 5; channel_1_range.has(address)) {
        channel_1_.on_write(static_cast<pulse_channel::register_index>(idx), data);
    } else if(channel_2_range.has(address)) {
        channel_2_.on_write(static_cast<pulse_channel::register_index>(idx), data);
    } else if(channel_3_range.has(address)) {
        channel_3_.on_write(static_cast<wave_channel::register_index>(idx), data);
    } else if(channel_4_range.has(address)) {
        channel_4_.on_write(static_cast<noise_channel::register_index>(idx), data);
    }

    else if(address == nr_50_addr) { control_.nr_50 = data; }
    else if(address == nr_51_addr) { control_.nr_51 = data; }
    else if(address == nr_52_addr) {
        if(!bit::test(data, 7u)) {
            std::for_each(begin(apu_register_addresses), end(apu_register_addresses) - 1, [&](const auto& addr) {
                on_write(addr, 0x00u);
            });

            channel_1_.disable();
            channel_2_.disable();
            channel_3_.disable();
            channel_4_.disable();

            power_on_ = false;
        } else if(!power_on_) {
            frame_sequencer_ = 0u;
            frame_sequencer_counter_ = frame_sequence_count;
            power_on_ = true;
        }
    }
}

uint8_t apu::on_read(const address16& address) const noexcept
{
    // ch1
    if(address == nr_10_addr) { return channel_1_.sweep.reg.value() | 0x80u; }
    if(address == nr_11_addr) { return channel_1_.wave_data.reg.value() | 0x3Fu; }
    if(address == nr_12_addr) { return channel_1_.envelope.reg.value(); }
    if(address == nr_14_addr) { return channel_1_.frequency_data.freq_control.reg.value() | 0xBFu; }

    // ch2
    if(address == nr_21_addr) { return channel_2_.wave_data.reg.value() | 0x3Fu; }
    if(address == nr_22_addr) { return channel_2_.envelope.reg.value(); }
    if(address == nr_24_addr) { return channel_2_.frequency_data.freq_control.reg.value() | 0xBFu; }

    // ch3
    if(address == nr_30_addr) { return (bit::from_bool(channel_3_.dac_enabled) << 7u) | 0x7Fu; }
    if(address == nr_32_addr) { return channel_3_.output_level.value() | 0x9Fu; }
    if(address == nr_34_addr) { return channel_3_.frequency.freq_control.reg.value() | 0xBFu; }

    // ch4
    if(address == nr_42_addr) { return channel_4_.envelope.reg.value(); }
    if(address == nr_43_addr) { return channel_4_.polynomial_counter.reg.value(); }
    if(address == nr_44_addr) { return channel_4_.control.reg.value() | 0xBFu; }

    // control
    if(address == nr_50_addr) { return control_.nr_50.value(); }
    if(address == nr_51_addr) { return control_.nr_51.value(); }
    if(address == nr_52_addr) {
        return 0x70u |
            (bit::from_bool(power_on_) << 7u) |
            (bit::from_bool(channel_4_.enabled) << 3u) |
            (bit::from_bool(channel_3_.enabled) << 2u) |
            (bit::from_bool(channel_2_.enabled) << 1u) |
            (bit::from_bool(channel_1_.enabled) << 0u);
    }

    return 0xFFu;
}

void apu::on_wave_pattern_write(const address16& address, const uint8_t data) noexcept
{
    channel_3_.wave_pattern[(address - *begin(wave_pattern_range)).value()] = data;
}

uint8_t apu::on_wave_pattern_read(const address16& address) const noexcept
{
    return channel_3_.wave_pattern[(address - *begin(wave_pattern_range)).value()];
}

} // namespace gameboy
