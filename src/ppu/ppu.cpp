#include <ppu/ppu.h>
#include <ppu/data/obj.h>
#include <ppu/data/bg.h>
#include <bus.h>
#include <cartridge.h>
#include <cpu/cpu.h>
#include <memory/mmu.h>
#include <memory/address.h>
#include <memory/memory_constants.h>
#include <util/mathutil.h>

namespace gameboy {

constexpr auto ly_max = 153;

constexpr auto hblank_cycles = 207;
constexpr auto vblank_cycles = 456;
constexpr auto reading_oam_cycles = 83;
constexpr auto reading_oam_vram_cycles = 175;

constexpr address16 lcdc_addr(0xFF40u);
constexpr address16 stat_addr(0xFF41u);

// Specifies the position in the 256x256 pixels BG map (32x32 tiles)
// which is to be displayed at the upper/left LCD display position.
// Values in range from 0-255 may be used for X/Y each, the video controller
// automatically wraps back to the upper (left) position in BG map when
// drawing exceeds the lower (right) border of the BG map area.
constexpr address16 scy_addr(0xFF42u);
constexpr address16 scx_addr(0xFF43u);

// The LY indicates the vertical line to which the present data is transferred
// to the LCD Driver. The LY can take on any value between 0 through 153.
// The values between 144 and 153 indicate the V-Blank period. Writing will reset the counter.
constexpr address16 ly_addr(0xFF44u);

// The gameboy permanently compares the value of the LYC and LY registers.
// When both values are identical, the coincident bit in the STAT register becomes set,
// and (if enabled) a STAT interrupt is requested.
constexpr address16 lyc_addr(0xFF45u);

// Specifies the upper/left positions of the Window area.
// (The window is an alternate background area which can be displayed above of the normal background.
// OBJs (sprites) may be still displayed above or behinf the window, just as for normal BG.)
// The window becomes visible (if enabled) when positions are set in range WX=0..166, WY=0..143.
// A postion of WX=7, WY=0 locates the window at upper left, it is then completly covering normal background.
constexpr address16 wy_addr(0xFF4Au);
constexpr address16 wx_addr(0xFF4Bu);

/*
 * This register assigns gray shades to the color numbers of the BG and Window tiles.
 *
 * Bit 7-6 - Shade for Color Number 3
 * Bit 5-4 - Shade for Color Number 2
 * Bit 3-2 - Shade for Color Number 1
 * Bit 1-0 - Shade for Color Number 0
 *
 * The four possible gray shades are:
 *
 * 0  White
 * 1  Light gray
 * 2  Dark gray
 * 3  Black
 *
 * In CGB Mode the Color Palettes are taken from CGB Palette Memory instead.
 */
constexpr address16 bgp_addr(0xFF47u);

// This register assigns gray shades for sprite palette 0.
// It works exactly as BGP (FF47), except that the lower
// two bits aren't used because sprite data 00 is transparent.
constexpr address16 obp_0_addr(0xFF48u);

//  This register assigns gray shades for sprite palette 1.
//  It works exactly as BGP (FF47), except that the lower
//  two bits aren't used because sprite data 00 is transparent.
constexpr address16 obp_1_addr(0xFF49u);

/*
 * This register is used to address a byte in the CGBs Background Palette Memory.
 * Each two byte in that memory define a color value.
 * The first 8 bytes define Color 0-3 of Palette 0 (BGP0), and so on for BGP1-7.
 *   Bit 0-5   Index (00-3F)
 *   Bit 7     Auto Increment  (0=Disabled, 1=Increment after Writing)
 *
 * Data can be read/written to/from the specified index address through Register FF69.
 * When the Auto Increment Bit is set then the index is automatically incremented after
 * each <write> to FF69. Auto Increment has no effect when <reading> from FF69,
 * so the index must be manually incremented in that case.
 */
constexpr address16 bgpi_addr(0xFF68u);

/*
 * This register allows to read/write data to the CGBs Background Palette Memory, addressed through Register FF68.
 * Each color is defined by two bytes (Bit 0-7 in first byte).
 *   Bit 0-4   Red Intensity   (00-1F)
 *   Bit 5-9   Green Intensity (00-1F)
 *   Bit 10-14 Blue Intensity  (00-1F)
 * Much like VRAM, Data in Palette Memory cannot be read/written during the
 * time when the LCD Controller is reading from it. (That is when the STAT register indicates Mode 3).
 * Note: Initially all background colors are initialized as white.
 */
constexpr address16 bgpd_addr(0xFF69u);

// These registers are used to initialize the Sprite Palettes OBP0-7,
// identically as described above for Background Palettes.
// Note that four colors may be defined for each OBP Palettes -
// but only Color 1-3 of each Sprite Palette can be displayed,
// Color 0 is always transparent, and can be initialized to a don't care value.
// Note: Initially all sprite colors are uninitialized.
constexpr address16 obpi_addr(0xFF6Au);
constexpr address16 obpd_addr(0xFF6Bu);

constexpr address16 vbk_addr(0xFF4Fu);

constexpr address16 oam_dma_addr(0xFF46u);

constexpr address16 hdma_1_addr(0xFF51u); // New DMA Source, High
constexpr address16 hdma_2_addr(0xFF52u); // New DMA Source, Low
constexpr address16 hdma_3_addr(0xFF53u); // New DMA Destination, High
constexpr address16 hdma_4_addr(0xFF54u); // New DMA Destination, Low
constexpr address16 hdma_5_addr(0xFF55u); // New DMA Length/Mode/Start

ppu::ppu(observer<bus> bus)
    : bus_{bus},
      ram_((bus->get_cartridge()->cgb_enabled() ? 2 : 1) * 8_kb, 0u),
      oam_(oam_range.size(), 0u)
{
    constexpr std::array dma_registers{
        oam_dma_addr,
        hdma_1_addr,
        hdma_2_addr,
        hdma_3_addr,
        hdma_4_addr,
        hdma_5_addr
    };
    for(const auto& addr : dma_registers) {
        bus->get_mmu()->add_memory_callback({
            addr,
            {connect_arg<&ppu::dma_read>, this},
            {connect_arg<&ppu::dma_write>, this}
        });
    }

    constexpr std::array general_purpose_registers{
        vbk_addr,
        lcdc_addr,
        stat_addr,
        scy_addr,
        scx_addr,
        ly_addr,
        lyc_addr,
        wy_addr,
        wx_addr
    };
    for(const auto& addr : general_purpose_registers) {
        bus->get_mmu()->add_memory_callback({
            addr,
            {connect_arg<&ppu::general_purpose_register_read>, this},
            {connect_arg<&ppu::general_purpose_register_write>, this}
        });
    }

    constexpr std::array palette_registers{
        bgp_addr,
        obp_0_addr,
        obp_1_addr,
        bgpi_addr,
        bgpd_addr,
        obpi_addr,
        obpd_addr
    };
    for(const auto& addr : palette_registers) {
        bus->get_mmu()->add_memory_callback({
            addr,
            {connect_arg<&ppu::palette_read>, this},
            {connect_arg<&ppu::palette_write>, this}
        });
    }
}

void ppu::tick(const uint8_t cycles)
{
    if(!lcdc_.lcd_enabled()) {
        return;
    }

    cycle_count_ += cycles;

    const auto has_elapsed = [&](auto cycles) {
        if(cycle_count_ >= cycles) {
            cycle_count_ -= cycles;
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

    return 0;
}

void ppu::write(const address16& address, const uint8_t data)
{
    if(vram_range.has(address)) {
        ram_[address.value() - *begin(vram_range) + vram_bank_ * 8_kb] = data;
    } else if(oam_range.has(address)) {
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
    if(address == bgp_addr) {
        return bgp_.value();
    }

    if(address == obp_0_addr) {
        return obp_0_.value();
    }

    if(address == obp_1_addr) {
        return obp_1_.value();
    }

    if(address == bgpi_addr) {
        return bgpi_.value();
    }

    if(address == obpi_addr) {
        return obpi_.value();
    }

    /*if(address == bgpd_addr) {
        //return scy_.value();
    }

    if(address == obpd_addr) {
        //return scy_.value();
    }*/

    return 0u;
}

void ppu::palette_write(const address16& address, const uint8_t data)
{
    if(address == bgp_addr) {
        bgp_ = data;
    } else if(address == obp_0_addr) {
        //return lcdc_.value();
    } else if(address == obp_1_addr) {
        //return stat_.value();
    } else if(address == bgpi_addr) {
        //return scy_.value();
    } else if(address == bgpd_addr) {
        //return scy_.value();
    } else if(address == obpi_addr) {
        //return scy_.value();
    } else if(address == obpd_addr) {
        //return scy_.value();
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

void ppu::render()
{
    render_line c{};
    on_render_line(ly_.value(), c);
}

} // namespace gameboy
