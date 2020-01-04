#include "gameboy/ppu/ppu.h"
#include "gameboy/ppu/data/bg.h"
#include "gameboy/bus.h"
#include "gameboy/cartridge.h"
#include "gameboy/cpu/cpu.h"
#include "gameboy/memory/mmu.h"
#include "gameboy/memory/address.h"
#include "gameboy/memory/memory_constants.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

constexpr auto ly_max = 153;

constexpr auto hblank_cycles = 207;
constexpr auto vblank_cycles = 456;
constexpr auto reading_oam_cycles = 83;
constexpr auto reading_oam_vram_cycles = 175;

constexpr address16 lcdc_addr{0xFF40u};
constexpr address16 stat_addr{0xFF41u};

// Specifies the position in the 256x256 pixels BG map (32x32 tiles)
// which is to be displayed at the upper/left LCD display position.
// Values in range from 0-255 may be used for X/Y each, the video controller
// automatically wraps back to the upper (left) position in BG map when
// drawing exceeds the lower (right) border of the BG map area.
constexpr address16 scy_addr{0xFF42u};
constexpr address16 scx_addr{0xFF43u};

// The gameboy permanently compares the value of the LYC and LY registers.
// When both values are identical, the coincident bit in the STAT register becomes set,
// and (if enabled) a STAT interrupt is requested.
constexpr address16 lyc_addr{0xFF45u};

// Specifies the upper/left positions of the Window area.
// (The window is an alternate background area which can be displayed above of the normal background.
// OBJs (sprites) may be still displayed above or behinf the window, just as for normal BG.)
// The window becomes visible (if enabled) when positions are set in range WX=0..166, WY=0..143.
// A postion of WX=7, WY=0 locates the window at upper left, it is then completly covering normal background.
constexpr address16 wy_addr{0xFF4Au};
constexpr address16 wx_addr{0xFF4Bu};

constexpr address16 bgp_addr{0xFF47u};
constexpr address16 obp_0_addr{0xFF48u};
constexpr address16 obp_1_addr{0xFF49u};
constexpr address16 bgpi_addr{0xFF68u};
constexpr address16 bgpd_addr{0xFF69u};
constexpr address16 obpi_addr{0xFF6Au};
constexpr address16 obpd_addr{0xFF6Bu};

constexpr address16 vbk_addr{0xFF4Fu};

constexpr address16 oam_dma_addr{0xFF46u};
constexpr address16 hdma_1_addr{0xFF51u}; // New DMA Source, High
constexpr address16 hdma_2_addr{0xFF52u}; // New DMA Source, Low
constexpr address16 hdma_3_addr{0xFF53u}; // New DMA Destination, High
constexpr address16 hdma_4_addr{0xFF54u}; // New DMA Destination, Low
constexpr address16 hdma_5_addr{0xFF55u}; // New DMA Length/Mode/Start

void set_palette(register8& index_register, std::array<palette, 8>& palettes, const uint8_t data) noexcept
{
    const auto auto_increment = bit_test(index_register, 7u);
    const auto is_msb = bit_test(index_register, 0u);
    const auto color_index = index_register.value() >> 1u & 0x03u;
    const auto palette_index = index_register.value() >> 3u & 0x07u;

    // msb | xBBBBBGG |
    // lsb | GGGRRRRR |
    auto& color = palettes[palette_index].colors[color_index];
    if(is_msb) {
        color.blue = data >> 2u & 0x1Fu;
        color.green |= (data & 0x03u) << 3u;
    } else {
        color.red = data & 0x1Fu;
        color.green = (data >> 5u) & 0x03u;
    }

    if(auto_increment) {
        index_register += 1u;
    }
}

template<auto ReadFunc, auto WriteFunc, typename... Registers>
void add_delegate(observer<bus> bus, ppu* p, Registers... registers)
{
    std::array dma_registers{registers...};
    for(const auto& addr : dma_registers) {
        bus->get_mmu()->add_memory_delegate({
            addr,
            {connect_arg<ReadFunc>, p},
            {connect_arg<WriteFunc>, p}
        });
    }
}

ppu::ppu(observer<bus> bus)
    : bus_{bus},
      cycle_count_{0u},
      vram_bank_{0u},
      ram_((bus->get_cartridge()->cgb_enabled() ? 2 : 1) * 8_kb, 0u),
      oam_(oam_range.size(), 0u),
      lcdc_{0x91u},
      stat_{0x05u},
      ly_{0x00u},
      lyc_{0x00u},
      scx_{0x00u},
      scy_{0x00u},
      wx_{0x00u},
      wy_{0x00u},
      bgp_{0xFCu},
      bgpi_{0x00u},
      obpi_{0x00u}
{
    const auto fill_palettes = [](auto& p, const auto& palette) { std::fill(begin(p), end(p), palette); };
    fill_palettes(obp_, register8{0xFFu});
    fill_palettes(cgb_bg_palettes_, palette{color{0xFFu}});
    fill_palettes(cgb_obj_palettes_, palette{color{0xFFu}});

    add_delegate<&ppu::dma_read, &ppu::dma_write>(bus, this,
        oam_dma_addr);
    add_delegate<&ppu::general_purpose_register_read, &ppu::general_purpose_register_write>(bus, this,
        lcdc_addr, stat_addr, scy_addr, scx_addr, ly_addr, lyc_addr, wy_addr, wx_addr);
    add_delegate<&ppu::palette_read, &ppu::palette_write>(bus, this,
        bgp_addr, obp_0_addr, obp_1_addr);

    if(bus->get_cartridge()->cgb_enabled()) {
        add_delegate<&ppu::dma_read, &ppu::dma_write>(bus, this,
            hdma_1_addr, hdma_2_addr, hdma_3_addr, hdma_4_addr, hdma_5_addr);

        add_delegate<&ppu::general_purpose_register_read, &ppu::general_purpose_register_write>(bus, this,
            vbk_addr);

        add_delegate<&ppu::palette_read, &ppu::palette_write>(bus, this,
            bgpi_addr, bgpd_addr, obpi_addr, obpd_addr);
    }
}

void ppu::tick(const uint8_t cycles)
{
    if(!lcdc_.lcd_enabled()) {
        return;
    }

    cycle_count_ += cycles;

    const auto has_elapsed = [&](const auto c) {
        if(cycle_count_ >= c) {
            cycle_count_ -= c;
            return true;
        }
        return false;
    };

    const auto update_ly = [&]() {
        ly_ = (ly_ + 1) % ly_max;
    };

    const auto compare_lyc = [&]() {
        if(ly_ == lyc_) {
            stat_.set_coincidence_flag();
        } else {
            stat_.reset_coincidence_flag();
        }

        if(stat_.coincidence_flag_set() && bus_->get_cpu()->interrupts_enabled()) {
            bus_->get_cpu()->request_interrupt(interrupt::lcd_stat);
        }
    };

    const auto check_stat_interrupt = [&]() {
        if(stat_.mode_interrupt_set() && bus_->get_cpu()->interrupts_enabled()) {
            bus_->get_cpu()->request_interrupt(interrupt::lcd_stat);
        }
    };

    switch(stat_.get_mode()) {
        case stat_mode::h_blank: {
            if(has_elapsed(hblank_cycles)) {
                update_ly();
                compare_lyc();

                if(ly_ == screen_height) {
                    on_render_frame();
                    stat_.set_mode(stat_mode::v_blank);
                    bus_->get_cpu()->request_interrupt(interrupt::lcd_vblank);
                } else {
                    stat_.set_mode(stat_mode::reading_oam);
                }

                check_stat_interrupt();
            }
            break;
        }
        case stat_mode::v_blank: {
            if(has_elapsed(vblank_cycles)) {
                update_ly();
                compare_lyc();

                if(ly_ == 0) {
                    stat_.set_mode(stat_mode::reading_oam);
                    check_stat_interrupt();
                }
            }
            break;
        }
        case stat_mode::reading_oam: {
            if(has_elapsed(reading_oam_cycles)) {
                stat_.set_mode(stat_mode::reading_oam_vram);
            }
            break;
        }
        case stat_mode::reading_oam_vram: {
            if(has_elapsed(reading_oam_vram_cycles)) {
                render();
                stat_.set_mode(stat_mode::h_blank);
                hdma();
                check_stat_interrupt();
            }
            break;
        }
    }
}

uint8_t ppu::read(const address16& address) const
{
    if(vram_range.has(address)) {
        if(stat_.get_mode() == stat_mode::reading_oam_vram) {
            return 0xFFu;
        }

        return ram_[address.value() - *begin(vram_range) + vram_bank_ * 8_kb];
    }

    if(oam_range.has(address)) {
        if(stat_.get_mode() == stat_mode::reading_oam || stat_.get_mode() == stat_mode::reading_oam_vram) {
            return 0xFFu;
        }

        return oam_[address.value() - *begin(oam_range)];
    }

    return 0u;
}

void ppu::write(const address16& address, const uint8_t data)
{
    if(vram_range.has(address)) {
        if(stat_.get_mode() == stat_mode::reading_oam_vram) {
            return;
        }

        ram_[address.value() - *begin(vram_range) + vram_bank_ * 8_kb] = data;
    } else if(oam_range.has(address)) {
        if(stat_.get_mode() == stat_mode::reading_oam || stat_.get_mode() == stat_mode::reading_oam_vram) {
            return;
        }

        oam_[address.value() - *begin(oam_range)] = data;
    }
}

uint8_t ppu::dma_read(const address16& address) const
{
    if(address == hdma_1_addr) { return (dma_transfer_.source.value() & 0xFF00u) >> 8u; }
    if(address == hdma_2_addr) { return dma_transfer_.source.value() & 0x00FFu; }
    if(address == hdma_3_addr) { return (dma_transfer_.destination.value() & 0xFF00u) >> 8u; }
    if(address == hdma_4_addr) { return dma_transfer_.destination.value() & 0x00FFu; }
    if(address == hdma_5_addr) { return dma_transfer_.length_mode_start.value(); }

    return 0u;
}

void ppu::dma_write(const address16& address, const uint8_t data)
{
    if(address == oam_dma_addr) {
        bus_->get_mmu()->dma(
            make_address(static_cast<uint16_t>(data << 8u)),
            make_address(*begin(oam_range)),
            oam_range.size() - 1);
    } else if(address == hdma_1_addr) {
        if(dma_transfer_.active()) {
            return;
        }

        dma_transfer_.source.low() = data & 0xF0u;
    } else if(address == hdma_2_addr) {
        if(dma_transfer_.active()) {
            return;
        }

        dma_transfer_.source.high() = data;
    } else if(address == hdma_3_addr) {
        dma_transfer_.destination.low() = data & 0xF0u;
    } else if(address == hdma_4_addr) {
        dma_transfer_.destination.high() = data & 0x1Fu;
    } else if(address == hdma_5_addr) {
        if(!bit_test(data, 7u)) {
            if(!dma_transfer_.active()) {
                bus_->get_mmu()->dma(
                    make_address(dma_transfer_.source),
                    make_address(dma_transfer_.destination),
                    dma_transfer_.length());
            } else {
                dma_transfer_.disable();
            }
        } else {
            dma_transfer_.remaining_length = dma_transfer_.length();
        }

        dma_transfer_.length_mode_start = data;
    }
}

uint8_t ppu::general_purpose_register_read(const address16& address) const
{
    if(address == vbk_addr) { return vram_bank_; }
    if(address == lcdc_addr) { return lcdc_.reg.value(); }
    if(address == stat_addr) { return stat_.reg.value(); }
    if(address == scy_addr) { return scy_.value(); }
    if(address == scx_addr) { return scx_.value(); }
    if(address == ly_addr) { return ly_.value(); }
    if(address == lyc_addr) { return lyc_.value(); }
    if(address == wy_addr) { return wy_.value(); }
    if(address == wx_addr) { return wx_.value(); }

    return 0u;
}

void ppu::general_purpose_register_write(const address16& address, const uint8_t data)
{
    if(address == vbk_addr) {
        if(dma_transfer_.active()) {
            return;
        }

        vram_bank_ = data & 0x01u;
    } else if(address == lcdc_addr) {
        if(bit_test(data, 7u) && !lcdc_.lcd_enabled()) {
            ly_ = 0;
            cycle_count_ = 0;
        }

        lcdc_.reg = data;
    } else if(address == stat_addr) {
        stat_.reg = data;
    } else if(address == scy_addr) {
        scy_ = data;
    } else if(address == scx_addr) {
        scx_ = data;
    } else if(address == ly_addr) {
        ly_ = 0;
    } else if(address == lyc_addr) {
        lyc_ = data;
    } else if(address == wy_addr) {
        wy_ = data;
    } else if(address == wx_addr) {
        wx_ = data;
    }
}

uint8_t ppu::palette_read(const address16& address) const
{
    if(address == bgp_addr) { return bgp_.value(); }
    if(address == obp_0_addr) { return obp_[0].value(); }
    if(address == obp_1_addr) { return obp_[1].value(); }
    if(address == bgpi_addr) { return bgpi_.value(); }
    if(address == obpi_addr) { return obpi_.value(); }

    return 0u;
}

void ppu::palette_write(const address16& address, const uint8_t data)
{
    if(address == bgp_addr) {
        bgp_ = data;
    } else if(address == obp_0_addr) {
        obp_[0] = data;
    } else if(address == obp_1_addr) {
        obp_[1] = data;
    } else if(address == bgpi_addr) {
        bgpi_ = data;
    } else if(address == obpi_addr) {
        obpi_ = data;
    } else if(address == bgpd_addr) {
        if(stat_.get_mode() == stat_mode::reading_oam_vram) {
            return;
        }

        set_palette(bgpi_, cgb_bg_palettes_, data);
    } else if(address == obpd_addr) {
        if(stat_.get_mode() == stat_mode::reading_oam_vram) {
            return;
        }

        set_palette(obpi_, cgb_obj_palettes_, data);
    }
}

void ppu::hdma()
{
    if(dma_transfer_.active() && ly_ < screen_height) {
        const auto total_length = dma_transfer_.length();
        const auto offset = total_length - dma_transfer_.remaining_length;

        bus_->get_mmu()->dma(
            make_address(static_cast<uint16_t>(dma_transfer_.source + offset)),
            make_address(static_cast<uint16_t>(dma_transfer_.destination + offset)),
            0x10u);

        dma_transfer_.remaining_length -= 0x10u;
        if(dma_transfer_.remaining_length == 0u) {
            dma_transfer_.disable();
        }
    }
}

void ppu::render() const noexcept
{
    render_line line{};
    std::fill(begin(line), end(line), color{0xFFu});

    // todo replace with render strategies
    if(bus_->get_cartridge()->cgb_enabled()) {
        if(lcdc_.bg_enabled()) { // bg enabled overrides window
            render_background();

            if(lcdc_.window_enabled() && ly_ >= wy_) {
                render_window();
            }
        }

        if(lcdc_.obj_enabled()) {
            render_obj();
        }
    } else {
        if(lcdc_.bg_enabled()) {
            render_background();
        }

        if(lcdc_.window_enabled() && ly_ >= wy_) {
            render_window();
        }

        if(lcdc_.obj_enabled()) {
            render_obj();
        } 
    }

    on_render_line(ly_.value(), line);
}

void ppu::render_background() const noexcept
{
    
}

void ppu::render_window() const noexcept
{
    
}

void ppu::render_obj() const noexcept
{
    
}

} // namespace gameboy
