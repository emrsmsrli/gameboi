#include "gameboy/apu/apu.h"
#include "gameboy/bus.h"
#include "gameboy/memory/mmu.h"
#include "gameboy/memory/memory_constants.h"

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

constexpr auto frame_sequence_count = 8192u;
constexpr auto frame_sequencer_max = 7u;
constexpr auto down_sample_count = 95u;

apu::apu(observer<bus> bus)
    : bus_{bus},
      enabled_{true},
      channel_1_{
          true,
          sweep{register8{0x80u}},
          wave_pattern_duty{register8{0xBFu}},
          envelope{register8{0xF3u}},
          frequency{
              register8{0x00u},
              register8{0xBFu}
          },
      },
      channel_2_{
          false,
          sweep{register8{0x00u}}, // no sweep
          wave_pattern_duty{register8{0x3Fu}},
          envelope{register8{0x00u}},
          frequency{
              register8{0x00u},
              register8{0xBFu}
          },
      },
      channel_3_{
          false,
          register8{0xFFu},
          register8{0x9Fu},
          frequency{
              register8{0x00u},
              register8{0xBFu}
          }
      },
      channel_4_{
          false,
          register8{0xFFu},
          envelope{register8{0x00u}},
          polynomial_counter{register8{0x00u}},
          register8{0xBFu}
      },
      nr_50_{0x77u},
      nr_51_{0xF3u},
      frame_sequencer_counter_{frame_sequence_count},
      buffer_fill_amount_{0u},
      frame_sequencer_{0u},
      down_sample_counter_{down_sample_count},
      sound_buffer_(sample_size)
{
    auto mmu = bus->get_mmu();
    constexpr std::array register_addresses{
        nr_10_addr, nr_11_addr, nr_12_addr, nr_13_addr, nr_14_addr,
        nr_21_addr, nr_22_addr, nr_23_addr, nr_24_addr,
        nr_30_addr, nr_31_addr, nr_32_addr, nr_33_addr, nr_34_addr,
        nr_41_addr, nr_42_addr, nr_43_addr, nr_44_addr,
        nr_50_addr, nr_51_addr, nr_52_addr
    };

    for(const auto& addr : register_addresses) {
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
                    channel_1_.env_click();
                    channel_2_.env_click();
                    channel_4_.env_click();
                    break;
                default:
                    break;
            }

            ++frame_sequencer_;
            if(frame_sequencer_ > frame_sequencer_max) {
                frame_sequencer_ = 0u;
            }
        }

        channel_1_.tick();
        channel_2_.tick();
        channel_3_.tick();
        channel_4_.tick();

        --down_sample_counter_;
        if(down_sample_counter_ == 0) {
            down_sample_counter_ = down_sample_count;

            // todo mix sounds

            if(buffer_fill_amount_ > sample_size) {
                buffer_fill_amount_ = 0u;
                on_buffer_full_(sound_buffer_);
                break;
            }
        }
    }
}

void apu::on_write(const address16& address, uint8_t data) noexcept
{
    const auto protect_write = [&](auto& reg, const auto data) {
        if(!enabled_) {
            return;
        }

        reg = data;
    };

    // ch1
    if(address == nr_10_addr) { protect_write(channel_1_.sweep.reg, data | 0x80u); }
    else if(address == nr_11_addr) { protect_write(channel_1_.wave_pattern_duty.reg, data); }
    else if(address == nr_12_addr) { protect_write(channel_1_.envelope.reg, data); }
    else if(address == nr_13_addr) { protect_write(channel_1_.frequency.low, data); }
    else if(address == nr_14_addr) { protect_write(channel_1_.frequency.high, data); }

    // ch2
    else if(address == nr_21_addr) { protect_write(channel_2_.wave_pattern_duty.reg, data); }
    else if(address == nr_22_addr) { protect_write(channel_2_.envelope.reg, data); }
    else if(address == nr_23_addr) { protect_write(channel_2_.frequency.low, data); }
    else if(address == nr_24_addr) { protect_write(channel_2_.frequency.high, data); }

    // ch3
    else if(address == nr_30_addr) { protect_write(channel_3_.enabled, bit::test(data, 7u)); }
    else if(address == nr_31_addr) { protect_write(channel_3_.sound_length, data); }
    else if(address == nr_32_addr) { protect_write(channel_3_.output_level, data | 0x9Fu); }
    else if(address == nr_33_addr) { protect_write(channel_3_.frequency.low, data); }
    else if(address == nr_34_addr) { protect_write(channel_3_.frequency.high, data); }

    // ch4
    else if(address == nr_41_addr) { protect_write(channel_4_.sound_length, data | 0xC0u); }
    else if(address == nr_42_addr) { protect_write(channel_4_.envelope.reg, data); }
    else if(address == nr_43_addr) { protect_write(channel_4_.polynomial_counter.reg, data); }
    else if(address == nr_44_addr) { protect_write(channel_4_.counter, data); }

    // control
    else if(address == nr_50_addr) { protect_write(nr_50_, data); }
    else if(address == nr_51_addr) { protect_write(nr_51_, data); }
    else if(address == nr_52_addr) {
        enabled_ = bit::test(data, 7u);
        if(!enabled_) {
            // todo reset all channels
        }
    }
}

uint8_t apu::on_read(const address16& address) const noexcept
{
    // ch1
    if(address == nr_10_addr) { return channel_1_.sweep.reg.value(); }
    if(address == nr_11_addr) { return channel_1_.wave_pattern_duty.reg.value() | 0x3Fu; }
    if(address == nr_12_addr) { return channel_1_.envelope.reg.value(); }
    if(address == nr_14_addr) { return channel_1_.frequency.high.value() | 0xBFu; }

    // ch2
    if(address == nr_21_addr) { return channel_2_.wave_pattern_duty.reg.value() | 0x3Fu; }
    if(address == nr_22_addr) { return channel_2_.envelope.reg.value(); }
    if(address == nr_24_addr) { return channel_2_.frequency.high.value() | 0xBFu; }

    // ch3
    if(address == nr_30_addr) { return (bit::from_bool(channel_3_.enabled) << 7u) | 0x7Fu; }
    if(address == nr_32_addr) { return channel_3_.output_level.value(); }
    if(address == nr_34_addr) { return channel_3_.frequency.high.value() | 0xBFu; }

    // ch4
    if(address == nr_41_addr) { return channel_4_.sound_length.value(); }
    if(address == nr_42_addr) { return channel_4_.envelope.reg.value(); }
    if(address == nr_43_addr) { return channel_4_.polynomial_counter.reg.value(); }
    if(address == nr_44_addr) { return channel_4_.counter.value() | 0xBFu; }

    // control
    if(address == nr_50_addr) { return nr_50_.value(); }
    if(address == nr_51_addr) { return nr_51_.value(); }
    if(address == nr_52_addr) {
        return 0x70u |
            (bit::from_bool(enabled_) << 7u) |
            (bit::from_bool(channel_4_.enabled) << 3u) |
            (bit::from_bool(channel_3_.enabled) << 2u) |
            (bit::from_bool(channel_2_.enabled) << 1u) |
            (bit::from_bool(channel_1_.enabled) << 0u);
    }

    return 0xFFu;
}

void apu::on_wave_pattern_write(const address16& address, uint8_t data) noexcept
{
    channel_3_.wave_pattern[(address - *begin(wave_pattern_range)).value()] = data;
}

uint8_t apu::on_wave_pattern_read(const address16& address) const noexcept
{
    return channel_3_.wave_pattern[(address - *begin(wave_pattern_range)).value()];
}

} // namespace gameboy
