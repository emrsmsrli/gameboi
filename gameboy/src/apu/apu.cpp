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

apu::apu(observer<bus> bus)
    : bus_{bus},
      nr_10_{0x80u},
      nr_11_{0xBFu},
      nr_12_{0xF3u},
      nr_14_{0xBFu},
      nr_21_{0x3Fu},
      nr_22_{0x00u},
      nr_24_{0xBFu},
      nr_30_{0x7Fu},
      nr_31_{0xFFu},
      nr_32_{0x9Fu},
      nr_33_{0xBFu},
      nr_41_{0xFFu},
      nr_42_{0x00u},
      nr_43_{0x00u},
      nr_44_{0xBFu},
      nr_50_{0x77u},
      nr_51_{0xF3u},
      nr_52_{0xF1u},
      wave_pattern_{}
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
    
}

void apu::on_write(const address16& address, uint8_t data) noexcept
{
    if(address == nr_10_addr) { nr_10_ = data; }
    else if(address == nr_11_addr) { nr_11_ = data; }
    else if(address == nr_12_addr) { nr_12_ = data; }
    else if(address == nr_13_addr) { nr_13_ = data; }
    else if(address == nr_14_addr) { nr_14_ = data; }
    else if(address == nr_21_addr) { nr_21_ = data; }
    else if(address == nr_22_addr) { nr_22_ = data; }
    else if(address == nr_23_addr) { nr_23_ = data; }
    else if(address == nr_24_addr) { nr_24_ = data; }
    else if(address == nr_30_addr) { nr_30_ = data; }
    else if(address == nr_31_addr) { nr_31_ = data; }
    else if(address == nr_32_addr) { nr_32_ = data; }
    else if(address == nr_33_addr) { nr_33_ = data; }
    else if(address == nr_34_addr) { nr_34_ = data; }
    else if(address == nr_41_addr) { nr_41_ = data; }
    else if(address == nr_42_addr) { nr_42_ = data; }
    else if(address == nr_43_addr) { nr_43_ = data; }
    else if(address == nr_44_addr) { nr_44_ = data; }
    else if(address == nr_50_addr) { nr_50_ = data; }
    else if(address == nr_51_addr) { nr_51_ = data; }
    else if(address == nr_52_addr) { nr_52_ = data; }
}

uint8_t apu::on_read(const address16& address) const noexcept
{
    if(address == nr_10_addr) { return nr_10_.value(); }
    if(address == nr_11_addr) { return nr_11_.value(); }
    if(address == nr_12_addr) { return nr_12_.value(); }
    if(address == nr_13_addr) { return nr_13_.value(); }
    if(address == nr_14_addr) { return nr_14_.value(); }
    if(address == nr_21_addr) { return nr_21_.value(); }
    if(address == nr_22_addr) { return nr_22_.value(); }
    if(address == nr_23_addr) { return nr_23_.value(); }
    if(address == nr_24_addr) { return nr_24_.value(); }
    if(address == nr_30_addr) { return nr_30_.value(); }
    if(address == nr_31_addr) { return nr_31_.value(); }
    if(address == nr_32_addr) { return nr_32_.value(); }
    if(address == nr_33_addr) { return nr_33_.value(); }
    if(address == nr_34_addr) { return nr_34_.value(); }
    if(address == nr_41_addr) { return nr_41_.value(); }
    if(address == nr_42_addr) { return nr_42_.value(); }
    if(address == nr_43_addr) { return nr_43_.value(); }
    if(address == nr_44_addr) { return nr_44_.value(); }
    if(address == nr_50_addr) { return nr_50_.value(); }
    if(address == nr_51_addr) { return nr_51_.value(); }
    if(address == nr_52_addr) { return nr_52_.value(); }

    return 0u;
}

void apu::on_wave_pattern_write(const address16& address, uint8_t data) noexcept
{
    wave_pattern_[(address - *begin(wave_pattern_range)).value()] = data;
}

uint8_t apu::on_wave_pattern_read(const address16& address) const noexcept
{
    return wave_pattern_[(address - *begin(wave_pattern_range)).value()];
}

} // namespace gameboy
