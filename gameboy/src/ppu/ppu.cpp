#include "gameboy/ppu/ppu.h"

#include <cstring>

#include "gameboy/bus.h"
#include "gameboy/cartridge.h"
#include "gameboy/cpu/cpu.h"
#include "gameboy/memory/memory_constants.h"
#include "gameboy/memory/mmu.h"
#include "gameboy/util/variantutil.h"

namespace gameboy {

constexpr auto ly_max = 153u;

constexpr auto lcd_enable_delay_frames = 3;
constexpr auto lcd_enable_delay_cycles = 244;
constexpr auto hblank_cycles = 204u;
constexpr auto reading_oam_cycles = 80u;
constexpr auto reading_oam_vram_render_cycles = 160u;
constexpr auto reading_oam_vram_cycles = 172u;
constexpr auto total_line_cycles = 456u;
constexpr auto total_vblank_cycles = total_line_cycles * 10u;
constexpr auto total_frame_cycles = total_line_cycles * (ly_max + 1);

constexpr address16 lcdc_addr{0xFF40u};
constexpr address16 stat_addr{0xFF41u};

constexpr address16 scy_addr{0xFF42u};
constexpr address16 scx_addr{0xFF43u};

constexpr address16 lyc_addr{0xFF45u};

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

template<auto ReadFunc, auto WriteFunc, typename... Registers>
void add_delegate(observer<bus> bus, ppu* p, Registers... registers)
{
    std::array dma_registers{registers...};
    for(const auto& addr : dma_registers) {
        bus->get_mmu()->add_memory_delegate(addr, {
            {connect_arg<ReadFunc>, p},
            {connect_arg<WriteFunc>, p}
        });
    }
}

ppu::ppu(const observer<bus> bus)
    : bus_{bus},
      cgb_enabled_{bus_->get_cartridge()->cgb_enabled()},
      lcd_enabled_{true},
      line_rendered_{false},
      vblank_line_{0},
      window_line_{0u},
      lcd_enable_delay_frame_count_{0},
      lcd_enable_delay_cycle_count_{0},
      cycle_count_{0u},
      secondary_cycle_count_{0u},
      vram_bank_{0u},
      ram_((cgb_enabled_ ? 2 : 1) * 8_kb, 0u),
      oam_(oam_range.size(), 0u),
      interrupt_request_{0u},
      lcdc_{0x91u},
      stat_(cgb_enabled_ ? 0x01u : 0x06u),
      ly_(cgb_enabled_ ? 0x90u : 0x00u),
      lyc_{0x00u},
      scx_{0x00u},
      scy_{0x00u},
      wx_{0x00u},
      wy_{0x00u},
      gb_palette_{palette_zelda},
      bgp_{0xFCu},
      bgpi_{0x00u},
      bgpd_{0x00u},
      obpi_{0x00u},
      obpd_{0x00u}
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

    if(cgb_enabled_) {
        add_delegate<&ppu::dma_read, &ppu::dma_write>(bus, this,
            hdma_1_addr, hdma_2_addr, hdma_3_addr, hdma_4_addr, hdma_5_addr);

        add_delegate<&ppu::general_purpose_register_read, &ppu::general_purpose_register_write>(bus, this,
            vbk_addr);

        add_delegate<&ppu::palette_read, &ppu::palette_write>(bus, this,
            bgpi_addr, bgpd_addr, obpi_addr, obpd_addr);

        dma_transfer_.oam_dma = 0x00u;
    }
}

void ppu::tick(const uint8_t cycles)
{
    cycle_count_ += cycles;

    if(!lcd_enabled_) {
        if(lcd_enable_delay_cycle_count_ > 0) {
            lcd_enable_delay_cycle_count_ -= cycles;

            if(lcd_enable_delay_cycle_count_ <= 0) {
                lcd_enable_delay_cycle_count_ = 0;
                lcd_enable_delay_frame_count_ = lcd_enable_delay_frames;
                vblank_line_ = 0;
                window_line_ = 0;
                ly_ = 0u;

                cycle_count_ = 0u;
                secondary_cycle_count_ = 0u;
                lcd_enabled_ = true;
                stat_.set_mode(stat_mode::h_blank);

                interrupt_request_.reset_all();
                if(stat_.mode_interrupt_enabled(stat_mode::reading_oam)) {
                    bus_->get_cpu()->request_interrupt(interrupt::lcd_stat);
                    interrupt_request_.set(interrupt_request::oam);
                }

                compare_coincidence();
            }
        } else if(cycle_count_ >= total_frame_cycles) {
            cycle_count_ -= total_frame_cycles;
            on_vblank_();
        }

        return;
    }

    const auto has_elapsed = [&](const auto cycle_count) {
        if(cycle_count_ >= cycle_count) {
            cycle_count_ -= cycle_count;
            return true;
        }
        return false;
    };

    switch(stat_.get_mode()) {
        case stat_mode::h_blank: {
            if(has_elapsed(hblank_cycles)) {
                set_ly(register8(ly_ + 1));
                stat_.set_mode(stat_mode::reading_oam);

                if(cgb_enabled_ && !dma_transfer_.disabled()) {
                    hdma();
                }

                reset_interrupt_requests({interrupt_request::v_blank, interrupt_request::oam});

                if(ly_ == screen_height) {
                    stat_.set_mode(stat_mode::v_blank);
                    secondary_cycle_count_ = cycle_count_;
                    vblank_line_ = 0;
                    window_line_ = 0;

                    bus_->get_cpu()->request_interrupt(interrupt::lcd_vblank);

                    if(stat_.mode_interrupt_enabled()) {
                        request_interrupt(interrupt_request::v_blank);
                    }

                    if(lcd_enable_delay_frame_count_ > 0) {
                        --lcd_enable_delay_frame_count_;
                    } else {
                        on_vblank_();
                    }
                } else {
                    if(stat_.mode_interrupt_enabled()) {
                        request_interrupt(interrupt_request::oam);
                    }
                }

                interrupt_request_.reset(interrupt_request::h_blank);
            }
            break;
        }
        case stat_mode::v_blank: {
            secondary_cycle_count_ += cycles;

            if(secondary_cycle_count_ >= total_line_cycles) {
                secondary_cycle_count_ -= total_line_cycles;

                ++vblank_line_;
                if(vblank_line_ < 10) {
                    set_ly(register8(ly_ + 1u));
                }
            }

            if(cycle_count_ >= 4104u && secondary_cycle_count_ >= 4u && ly_ == ly_max) {
                set_ly(register8{0u});
            }

            if(has_elapsed(total_vblank_cycles)) {
                stat_.set_mode(stat_mode::reading_oam);

                reset_interrupt_requests({interrupt_request::h_blank, interrupt_request::oam});
                if(stat_.mode_interrupt_enabled()) {
                    request_interrupt(interrupt_request::oam);
                }

                interrupt_request_.reset(interrupt_request::v_blank);
            }
            break;
        }
        case stat_mode::reading_oam: {
            if(has_elapsed(reading_oam_cycles)) {
                stat_.set_mode(stat_mode::reading_oam_vram);
                line_rendered_ = false;

                reset_interrupt_requests({
                    interrupt_request::h_blank,
                    interrupt_request::v_blank,
                    interrupt_request::oam
                });
            }
            break;
        }
        case stat_mode::reading_oam_vram: {
            if(cycle_count_ >= reading_oam_vram_render_cycles && !line_rendered_) {
                line_rendered_ = true;
                render();
            }

            if(has_elapsed(reading_oam_vram_cycles)) {
                stat_.set_mode(stat_mode::h_blank);

                reset_interrupt_requests({
                    interrupt_request::h_blank,
                    interrupt_request::v_blank,
                    interrupt_request::oam
                });

                if(stat_.mode_interrupt_enabled()) {
                    request_interrupt(interrupt_request::h_blank);
                }
            }
            break;
        }
    }
}

uint8_t ppu::read_ram(const address16& address) const
{
    if(stat_.get_mode() == stat_mode::reading_oam_vram) {
        return 0xFFu;
    }

    return read_ram_by_bank(address, vram_bank_);
}

void ppu::write_ram(const address16& address, const uint8_t data)
{
    if(stat_.get_mode() == stat_mode::reading_oam_vram) {
        return;
    }

    write_ram_by_bank(address, data, vram_bank_);
}

uint8_t ppu::read_oam(const address16& address) const
{
    if(stat_.get_mode() == stat_mode::reading_oam || stat_.get_mode() == stat_mode::reading_oam_vram) {
        return 0xFFu;
    }

    return oam_[address.value() - *begin(oam_range)];
}

void ppu::write_oam(const address16& address, const uint8_t data)
{
    if(stat_.get_mode() == stat_mode::reading_oam || stat_.get_mode() == stat_mode::reading_oam_vram) {
        return;
    }

    oam_[address.value() - *begin(oam_range)] = data;
}

uint8_t ppu::read_ram_by_bank(const address16& address, const uint8_t bank) const
{
    if(!cgb_enabled_ && bank != 0u) {
        return 0x00u;
    }

    return ram_[address.value() - *begin(vram_range) + bank * 8_kb];
}

void ppu::write_ram_by_bank(const address16& address, const uint8_t data, const uint8_t bank)
{
    if(!cgb_enabled_ && bank != 0u) {
        return;
    }

    ram_[address.value() - *begin(vram_range) + bank * 8_kb] = data;
}

uint8_t ppu::dma_read(const address16& address) const
{
    if(address == oam_dma_addr) { return dma_transfer_.oam_dma.value(); }
    if(address == hdma_1_addr) { return dma_transfer_.source.high().value(); }
    if(address == hdma_2_addr) { return dma_transfer_.source.low().value(); }
    if(address == hdma_3_addr) { return dma_transfer_.destination.high().value(); }
    if(address == hdma_4_addr) { return dma_transfer_.destination.low().value(); }
    if(address == hdma_5_addr) { return dma_transfer_.length_mode_start.value(); }

    return 0u;
}

void ppu::dma_write(const address16& address, const uint8_t data)
{
    if(address == oam_dma_addr) {
        dma_transfer_.oam_dma = data;

        auto mmu = bus_->get_mmu();
        auto ptr = static_cast<uint16_t>(data) << 8u;
        for(auto& data : oam_) {
            data = mmu->read(ptr++);
        }
    } else if(address == hdma_1_addr) {
        if((data > 0x7Fu && data < 0xA0u) || data > 0xDFu) {
            dma_transfer_.source.high() = 0x00u;
        } else {
            dma_transfer_.source.high() = data;
        }
    } else if(address == hdma_2_addr) {
        dma_transfer_.source.low() = data & 0xF0u;
    } else if(address == hdma_3_addr) {
        dma_transfer_.destination.high() = (data & 0x1Fu) | 0x80u;
    } else if(address == hdma_4_addr) {
        dma_transfer_.destination.low() = data & 0xF0u;
    } else if(address == hdma_5_addr) {
        if(!dma_transfer_.disabled()) {
            if(bit::test(data, 7u)) {
                dma_transfer_.length_mode_start = data & 0x7Fu;
            } else {
                dma_transfer_.length_mode_start = data;
            }
        } else {
            if(bit::test(data, 7u)) {
                dma_transfer_.length_mode_start = data & 0x7Fu;
                if(stat_.get_mode() == stat_mode::h_blank) {
                    hdma();
                }
            } else {
                dma_transfer_.length_mode_start = data;
                gdma();
            }
        }
    }
}

uint8_t ppu::general_purpose_register_read(const address16& address) const
{
    if(address == vbk_addr) { return vram_bank_ | 0xFEu; }
    if(address == lcdc_addr) { return lcdc_.reg.value(); }
    if(address == stat_addr) { return stat_.reg.value() | 0x80u; }
    if(address == scy_addr) { return scy_.value(); }
    if(address == scx_addr) { return scx_.value(); }
    if(address == ly_addr) { return lcd_enabled_ ? ly_.value() : 0u; }
    if(address == lyc_addr) { return lyc_.value(); }
    if(address == wy_addr) { return wy_.value(); }
    if(address == wx_addr) { return wx_.value(); }

    return 0u;
}

void ppu::general_purpose_register_write(const address16& address, const uint8_t data)
{
    if(address == vbk_addr) {
        vram_bank_ = data & 0x01u;
    } else if(address == lcdc_addr) {
        register_lcdc new_lcdc{data};

        if(!lcdc_.window_enabled() && new_lcdc.window_enabled()) {
            if(window_line_ == 0u && ly_ < screen_height && ly_ > wy_) {
                window_line_ = screen_height;
            }
        }

        if(new_lcdc.lcd_enabled()) {
            if(!lcd_enabled_) {
                lcd_enable_delay_cycle_count_ = lcd_enable_delay_cycles;
            }
        } else {
            disable_screen();
        }

        lcdc_.reg = data;
    } else if(address == stat_addr) {
        stat_.reg = (stat_.reg & 0x07u) | (data & 0x78u);

        auto irq_copy = interrupt_request_;
        irq_copy.request &= (stat_.reg.value() >> 3u) & 0x0Fu;
        interrupt_request_ = irq_copy;

        if(lcdc_.lcd_enabled()) {
            if(stat_.mode_interrupt_enabled()) {
                switch(stat_.get_mode()) {
                    case stat_mode::h_blank:
                        request_interrupt(irq_copy, interrupt_request::h_blank);
                        break;
                    case stat_mode::v_blank:
                        request_interrupt(irq_copy, interrupt_request::v_blank);
                        break;
                    case stat_mode::reading_oam:
                        request_interrupt(irq_copy, interrupt_request::oam);
                        break;
                    case stat_mode::reading_oam_vram:
                        break;
                }
            }

            compare_coincidence();
        }
    } else if(address == scy_addr) {
        scy_ = data;
    } else if(address == scx_addr) {
        scx_ = data;
    } else if(address == ly_addr) {
        set_ly(register8{0x00u});
    } else if(address == lyc_addr) {
        set_lyc(register8{data});
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
    if(address == bgpd_addr) { return bgpd_.value(); }
    if(address == obpi_addr) { return obpi_.value(); }
    if(address == obpd_addr) { return obpd_.value(); }

    return 0u;
}

void ppu::palette_write(const address16& address, const uint8_t data)
{
    const auto update_palette_data_register = [](auto& index_register, auto& data_register, auto& palettes) {
        const auto is_msb = bit::test(index_register, 0u);
        const auto color_index = (index_register.value() >> 1u) & 0x03u;
        const auto palette_index = (index_register.value() >> 3u) & 0x07u;

        auto& color = palettes[palette_index].colors[color_index];
        if(is_msb) {
            data_register = (color.blue << 2u) | ((color.green >> 3u) & 0x03u);
        } else {
            data_register = ((color.green & 0x07u) << 5u) | color.red;
        }
    };

    const auto set_palette = [&](auto& index_register, auto& data_register, auto& palettes, const uint8_t data) noexcept {
        const auto auto_increment = bit::test(index_register, 7u);
        const auto is_msb = bit::test(index_register, 0u);
        const auto color_index = (index_register.value() >> 1u) & 0x03u;
        const auto palette_index = (index_register.value() >> 3u) & 0x07u;

        // msb | xBBBBBGG |
        // lsb | GGGRRRRR |
        auto& color = palettes[palette_index].colors[color_index];
        if(is_msb) {
            color.blue = (data >> 2u) & 0x1Fu;
            color.green |= (data & 0x03u) << 3u;
        } else {
            color.red = data & 0x1Fu;
            color.green = (data >> 5u) & 0x07u;
        }

        if(auto_increment) {
            index_register = (index_register & 0x80u) | ((index_register + 1u) & 0x3Fu);
            update_palette_data_register(index_register, data_register, palettes);
        }
    };

    if(address == bgp_addr) {
        bgp_ = data;
    } else if(address == obp_0_addr) {
        obp_[0] = data;
    } else if(address == obp_1_addr) {
        obp_[1] = data;
    } else if(address == bgpi_addr) {
        bgpi_ = data;
        update_palette_data_register(bgpi_, bgpd_, cgb_bg_palettes_);
    } else if(address == obpi_addr) {
        obpi_ = data;
        update_palette_data_register(obpi_, obpd_, cgb_obj_palettes_);
    } else if(address == bgpd_addr) {
        set_palette(bgpi_, bgpd_, cgb_bg_palettes_, data);
    } else if(address == obpd_addr) {
        set_palette(obpi_, obpd_, cgb_obj_palettes_, data);
    }
}

void ppu::compare_coincidence() noexcept
{
    if(ly_ == lyc_) {
        stat_.set_coincidence_flag();

        if(stat_.coincidence_interrupt_enabled()) {
            request_interrupt(interrupt_request::coincidence);
        }
    } else {
        stat_.reset_coincidence_flag();
        interrupt_request_.reset(interrupt_request::coincidence);
    }
}

void ppu::set_ly(const register8& ly) noexcept
{
    ly_ = ly;
    compare_coincidence();
}

void ppu::set_lyc(const register8& lyc) noexcept
{
    if(lyc_ != lyc) {
        lyc_ = lyc;
        if(lcdc_.lcd_enabled()) {
            compare_coincidence();
        }
    }
}

void ppu::disable_screen() noexcept
{
    lcd_enabled_ = false;
    stat_.set_mode(stat_mode::h_blank);
    interrupt_request_.reset_all();

    cycle_count_ = 0;
    secondary_cycle_count_ = 0;
    ly_ = 0;
}

void ppu::request_interrupt(interrupt_request::type type) noexcept
{
    request_interrupt(interrupt_request_, type);
}

void ppu::request_interrupt(interrupt_request& irq, interrupt_request::type type) noexcept
{
    if(irq.none()) {
        bus_->get_cpu()->request_interrupt(interrupt::lcd_stat);
    }

    irq.set(type);
}

void ppu::reset_interrupt_requests(const std::initializer_list<interrupt_request::type> irqs) noexcept
{
    for(const auto irq : irqs) {
        interrupt_request_.reset(irq);
    }
}

void ppu::hdma()
{
    bus_->get_mmu()->dma(
        make_address(dma_transfer_.source),
        make_address(dma_transfer_.destination),
        dma_transfer_data::unit_transfer_length);

    dma_transfer_.source += dma_transfer_data::unit_transfer_length;
    if(dma_transfer_.source == 0x8000u) {
        dma_transfer_.source = 0xA000u;
    }

    dma_transfer_.destination += dma_transfer_data::unit_transfer_length;
    if(dma_transfer_.destination == 0xA000u) {
        dma_transfer_.destination = 0x8000u;
    }

    dma_transfer_.length_mode_start -= 1u;
}

void ppu::gdma()
{
    const auto transfer_length = dma_transfer_.length();

    bus_->get_mmu()->dma(
        make_address(dma_transfer_.source),
        make_address(dma_transfer_.destination),
      transfer_length);

    dma_transfer_.source += transfer_length;
    dma_transfer_.destination += transfer_length;
    dma_transfer_.length_mode_start = 0xFFu;
}

void ppu::render() noexcept
{
    render_line line{};
    render_buffer buffer{};
    std::fill(begin(buffer), end(buffer), std::make_pair(0u, attributes::uninitialized{}));

    render_background(buffer);
    render_window(buffer);
    render_obj(buffer);

    for(auto pixel_idx = 0u; pixel_idx < line.size(); ++pixel_idx) {
        const auto& [color_idx, attr] = buffer[pixel_idx];

        visit_nt(attr,
            [&](attributes::uninitialized) {
                line[pixel_idx] = color{0xFFu};
            },
            [&, color = color_idx](const attributes::bg& bg_attr) {
                if(cgb_enabled_) {
                    const auto palette_color = cgb_bg_palettes_[bg_attr.palette_index()].colors[color];
                    line[pixel_idx] = correct_color(palette_color);
                } else {
                    const auto background_palette = palette::from(gb_palette_, bgp_.value());
                    line[pixel_idx] = background_palette.colors[color];
                }
            },
            [&, color = color_idx](const attributes::obj& obj_attr) {
                if(cgb_enabled_) {
                    const auto palette_color = cgb_obj_palettes_[obj_attr.cgb_palette_index()].colors[color];
                    line[pixel_idx] = correct_color(palette_color);
                } else {
                    const auto background_palette =
                        palette::from(gb_palette_, obp_[obj_attr.gb_palette_index()].value());
                    line[pixel_idx] = background_palette.colors[color];
                }
            }
        );
    }

    on_render_line_(ly_.value(), line);
}

void ppu::render_background(render_buffer& buffer) const noexcept
{
    if(!cgb_enabled_ && !lcdc_.bg_enabled()) {
        return;
    }

    const auto scroll_offset = (scx_ + screen_width) % map_pixel_count;
    const auto tile_start_addr = address16(lcdc_.bg_map_secondary() ? 0x9C00u : 0x9800u);
    const auto tile_y_to_render = (scy_ + ly_.value()) % tile_pixel_count;
    const auto tile_map_y = ((scy_ + ly_.value()) / tile_pixel_count) % map_tile_count;

    constexpr auto max_render_tile_count = screen_width / tile_pixel_count + 1;
    const auto tile_map_x_start = scx_.value() / tile_pixel_count;

    auto rendered_pix_idx = 0u;
    for(auto tile_render_count = 0u; tile_render_count < max_render_tile_count; ++tile_render_count) {
        const auto tile_map_x = (tile_map_x_start + tile_render_count) % map_tile_count;
        const auto tile_map_idx = tile_start_addr + tile_map_y * map_tile_count + tile_map_x;

        const auto tile_no = read_ram_by_bank(tile_map_idx, 0);
        const attributes::bg tile_attr{read_ram_by_bank(tile_map_idx, 1)};

        const auto tile_y = tile_attr.v_flipped()
            ? tile_pixel_count - tile_y_to_render - 1u
            : tile_y_to_render;

        auto tile_row = get_tile_row(tile_y, tile_no, tile_attr.vram_bank());
        if(tile_attr.h_flipped()) {
            std::reverse(begin(tile_row), end(tile_row));
        }

        for(auto tile_x = 0u; tile_x < tile_pixel_count; ++tile_x) {
            const auto pix_idx = tile_map_x * tile_pixel_count + tile_x;

            if(rendered_pix_idx < screen_width && (scx_ <= pix_idx ||
                (scx_ > scroll_offset && pix_idx < scroll_offset)) // bg scroll overflow
            ) {
                buffer[rendered_pix_idx++] = std::make_pair(tile_row[tile_x], tile_attr);
            }
        }
    }
}

void ppu::render_window(render_buffer& buffer) noexcept
{
    if(window_line_ >= screen_height || !lcdc_.window_enabled()) {
        return;
    }

    if(wy_ > ly_ || wy_ >= screen_height || wx_ >= screen_width + 7u) {
        return;
    }

    const auto tile_start_addr = address16(lcdc_.window_map_secondary() ? 0x9C00u : 0x9800u);
    const auto tile_y_to_render = window_line_ % tile_pixel_count;

    const auto tile_map_y_start = tile_start_addr + window_line_ / tile_pixel_count * map_tile_count;
    const auto tile_map_x_end = screen_width / tile_pixel_count;

    for(auto tile_map_x = 0; tile_map_x < tile_map_x_end + 1u; ++tile_map_x) {
        const auto tile_map_idx = tile_map_y_start + tile_map_x;
        const auto tile_no = read_ram_by_bank(tile_map_idx, 0);
        const attributes::bg tile_attr{read_ram_by_bank(tile_map_idx, 1)};

        const auto tile_y = tile_attr.v_flipped()
            ? tile_pixel_count - tile_y_to_render - 1u
            : tile_y_to_render;

        auto tile_row = get_tile_row(tile_y, tile_no, tile_attr.vram_bank());
        if(tile_attr.h_flipped()) {
            std::reverse(begin(tile_row), end(tile_row));
        }

        for(auto tile_x = 0u; tile_x < tile_pixel_count; ++tile_x) {
            const int16_t pix_idx = tile_map_x * tile_pixel_count + tile_x + wx_.value() - 7;

            if(pix_idx >= static_cast<int32_t>(screen_width)) {
                ++window_line_;
                return;
            }

            if(0 <= pix_idx) {
                buffer[pix_idx] = std::make_pair(tile_row[tile_x], tile_attr);
            }
        }
    }
}

void ppu::render_obj(render_buffer& buffer) const noexcept
{
    if(!lcdc_.obj_enabled()) {
        return;
    }

    const auto obj_size = lcdc_.large_obj() ? 16 : 8;

    std::array<attributes::obj, 40> objs;
    std::memcpy(&objs, oam_.data(), oam_.size());

    auto indices = [&]() {
        const int32_t ly = ly_.value();
        std::vector<size_t> idxs;

        for(size_t idx = 0; idx < objs.size(); ++idx) {
            const auto& obj = objs[idx];
            const auto obj_y = obj.y - 16;

            if(ly >= obj_y && ly < obj_y + obj_size) {
                idxs.push_back(idx);
                if(idxs.size() == 10) {
                    break;
                }
            }
        }

        return idxs;
    }();

    if(!cgb_enabled_) {
        std::sort(begin(indices), end(indices), [&](const auto l, const auto r) {
            const auto& obj_l = objs[l];
            const auto& obj_r = objs[r];

            if(obj_l.x == obj_r.x) {
                return l < r;
            }

            return obj_l.x < obj_r.x;
        });
    }

    std::reverse(begin(indices), end(indices));

    for(const auto index : indices) {
        const auto& obj = objs[index];

        const auto obj_y = obj.y - 16;
        const auto obj_x = obj.x - 8;

        if(-7 > obj_x || obj_x >= static_cast<int32_t>(screen_width)) {
            continue;
        }

        auto tile_row = get_tile_row(
            obj.v_flipped()
                ? obj_size - (ly_ - obj_y) - 1u
                : ly_ - obj_y,
            tile_address<uint8_t>(0x8000u, lcdc_.large_obj()
                ? obj.tile_number & 0xFEu
                : obj.tile_number),
            obj.vram_bank());

        if(obj.h_flipped()) {
            std::reverse(begin(tile_row), end(tile_row));
        }

        for(auto tile_x = 0u; tile_x < tile_pixel_count; ++tile_x) {
            const auto x = obj_x + tile_x;
            if(0u > x || x >= screen_width) {
                continue;
            }

            const auto& [color_idx, attr] = buffer[x];

            const auto dot_color = tile_row[tile_x];
            if(dot_color == 0u) { // obj color0 is transparent
                continue;
            }

            visit_nt(attr,
                [&](auto&&) {
                    buffer[x] = std::make_pair(dot_color, obj);
                },
                [&, existing_bg_color = color_idx](const attributes::bg& bg_attr) {
                    const auto master_priority_enabled = cgb_enabled_ && !lcdc_.bg_enabled();
                    const auto obj_priority_enabled = obj.prioritized() && !bg_attr.prioritized();

                    if(existing_bg_color == 0u || master_priority_enabled || obj_priority_enabled) {
                        buffer[x] = std::make_pair(dot_color, obj);
                    }
                }
            );
        }
    }
}

std::array<uint8_t, ppu::tile_pixel_count> ppu::get_tile_row(
    const uint8_t row, const uint8_t tile_no, const uint8_t bank) const noexcept
{
    const auto tile_base_addr = lcdc_.unsigned_mode()
        ? tile_address<uint8_t>(0x8000u, tile_no)
        : tile_address<int8_t>(0x9000u, tile_no);

    return get_tile_row(row, tile_base_addr, bank);
}

std::array<uint8_t, ppu::tile_pixel_count> ppu::get_tile_row(
    const uint8_t row,
    const address16& tile_base_addr,
    const uint8_t bank) const noexcept
{
    const auto tile_y_offset = row * 2;
    const auto lsb = read_ram_by_bank(tile_base_addr + tile_y_offset, bank);
    const auto msb = read_ram_by_bank(tile_base_addr + tile_y_offset + 1, bank);

    struct pixel_generator {
        uint8_t lsb;
        uint8_t msb;
        uint8_t bit = tile_pixel_count - 1;

        [[nodiscard]] uint8_t operator()() {
            const auto mask = static_cast<uint32_t>(1u << bit);
            const auto pix_color = static_cast<uint8_t>((msb & mask) >> bit << 1u | (lsb & mask) >> bit);
            --bit;
            return pix_color;
        }
    };

    std::array<uint8_t, tile_pixel_count> tile_row{};
    std::generate(begin(tile_row), end(tile_row), pixel_generator{lsb, msb});

    return tile_row;
}

color ppu::correct_color(const color& c) noexcept
{
    const auto do_correct = [](const uint8_t color) {
        return static_cast<uint8_t>(color * 0xFFu / 0x1Fu);
    };

    return {
        do_correct(c.red),
        do_correct(c.green),
        do_correct(c.blue)
    };
}

} // namespace gameboy
