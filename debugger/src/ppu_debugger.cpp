#include "debugger/ppu_debugger.h"

#include <cstring>

#include <SFML/Graphics/Sprite.hpp>

#include "gameboy/bus.h"
#include "gameboy/cartridge.h"
#include "gameboy/memory/memory_constants.h"
#include "gameboy/ppu/ppu.h"
#include "imgui-SFML.h"
#include "imgui.h"

namespace {
constexpr auto bg_map_tile_count = 32;
constexpr auto tiles_per_row = 16;
constexpr auto tile_row_count = 8;
}

gameboy::ppu_debugger::ppu_debugger(const observer<ppu> ppu) noexcept
    : ppu_{ppu}
{
    constexpr auto tile_area_count = 3;
    tiles_img_.create(tiles_per_row * ppu::tile_pixel_count, tile_row_count * ppu::tile_pixel_count * tile_area_count);
    tiles_.create(tiles_per_row * ppu::tile_pixel_count, tile_row_count * ppu::tile_pixel_count * tile_area_count);

    bg_map_.create(bg_map_tile_count * ppu::tile_pixel_count, bg_map_tile_count * ppu::tile_pixel_count);
    bg_map_img_.create(bg_map_tile_count * ppu::tile_pixel_count, bg_map_tile_count * ppu::tile_pixel_count);

    for(size_t i = 0u; i < 40u; ++i) {
        oam_imgs_[i].create(ppu::tile_pixel_count, ppu::tile_pixel_count * 2, sf::Color::White);
        oam_[i].create(ppu::tile_pixel_count, ppu::tile_pixel_count * 2);
    }
}

void gameboy::ppu_debugger::draw() noexcept
{
    if(!ImGui::Begin("PPU")) {
        ImGui::End();
        return;
    }

    if(ImGui::BeginTabBar("tabs")) {
        if(ImGui::BeginTabItem("Registers")) {
            draw_registers();
            ImGui::Separator();

            draw_lcdc_n_stat();
            ImGui::Separator();

            ImGui::Spacing();
            ImGui::Spacing();

            draw_dma();

            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("VRAM View")) {
            draw_vram_view();

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void gameboy::ppu_debugger::draw_registers() const noexcept
{
    ImGui::Columns(2, "registers", true);

    ImGui::Text("General"); ImGui::NextColumn();
    ImGui::Text("Window"); ImGui::NextColumn();

    ImGui::Separator();

    ImGui::Text("VBK:  %02X", ppu_->vram_bank_);
    ImGui::Text("LCDC: %02X", ppu_->lcdc_.reg.value());
    ImGui::Text("STAT: %02X", ppu_->stat_.reg.value());
    
    ImGui::NextColumn();

    ImGui::Text("LY:   %02X", ppu_->ly_.value());
    ImGui::Text("LYC:  %02X", ppu_->lyc_.value());
    ImGui::Text("SCX:  %02X", ppu_->scx_.value());
    ImGui::Text("SCY:  %02X", ppu_->scy_.value());
    ImGui::Text("WX:   %02X", ppu_->wx_.value());
    ImGui::Text("WY:   %02X", ppu_->wy_.value());

    ImGui::Columns(1);
}

void gameboy::ppu_debugger::draw_lcdc_n_stat() const
{
    ImGui::Columns(2, "lcdcnstat", true);

    ImGui::Text("LCDC"); ImGui::NextColumn();
    ImGui::Text("STAT"); ImGui::NextColumn();

    ImGui::Separator();
    
    ImGui::Text("LCD:  %s", ppu_->lcdc_.lcd_enabled() ? "on" : "off");
    ImGui::Text("WIN:  %04X", ppu_->lcdc_.window_map_secondary() ? 0x9C00u : 0x9800u);
    ImGui::Text("WIN:  %s", ppu_->lcdc_.window_enabled() ? "on" : "off");
    ImGui::Text("CHR:  %04X", ppu_->lcdc_.unsigned_mode() ? 0x8800u : 0x8000u);
    ImGui::Text("BG:   %04X", ppu_->lcdc_.bg_map_secondary() ? 0x9C00u : 0x9800u);
    ImGui::Text("OBJ:  %s", ppu_->lcdc_.large_obj() ? "8x16" : "8x8");
    ImGui::Text("OBJ:  %s" ,ppu_->lcdc_.obj_enabled() ? "on" : "off");
    ImGui::Text("BG:   %s", ppu_->lcdc_.bg_enabled() ? "on" : "off");

    ImGui::NextColumn();

    ImGui::Text("<LYC=LY>: %d", bit::test(ppu_->stat_.reg, 6));
    ImGui::Text("<OAM>:    %d", bit::test(ppu_->stat_.reg, 5));
    ImGui::Text("<VBlank>: %d", bit::test(ppu_->stat_.reg, 4));
    ImGui::Text("<HBlank>: %d", bit::test(ppu_->stat_.reg, 3));
    ImGui::Text("LYC=LY:   %d", bit::test(ppu_->stat_.reg, 2));
    ImGui::Text("mode:     %d (%s)", static_cast<int8_t>(ppu_->stat_.get_mode()), [&]() {
        switch(ppu_->stat_.get_mode()) {
            case stat_mode::h_blank: return "hblank";
            case stat_mode::v_blank: return "vblank";
            case stat_mode::reading_oam: return "reading oam";
            case stat_mode::reading_oam_vram: return "reading oam vram";
        }
    }());
    
    ImGui::Columns(1);
}

void gameboy::ppu_debugger::draw_palettes() const
{
    ImGui::SetNextItemOpen(true, 0);
    if(ImGui::TreeNode("GB")) {
        const auto draw_gb_palette = [&](auto name, auto data) {
            ImGui::SetNextItemOpen(true, 0);
            if(ImGui::TreeNode(name, "%s: %02X", name, data)) {
                for(const auto& color : palette::from(ppu_->gb_palette_, data).colors) {
                    const ImVec4 c{
                        color.red / 255.f,
                        color.green / 255.f,
                        color.blue / 255.f,
                        1.f
                    };

                    ImGui::ColorButton("", c, 0, ImVec2(20,20));
                    ImGui::SameLine(0, 10.f);
                }

                ImGui::NewLine();
                ImGui::TreePop();
            }
        };

        draw_gb_palette("BGP", ppu_->bgp_.value());
        draw_gb_palette("OBP1", ppu_->obp_[0].value());
        draw_gb_palette("OBP2", ppu_->obp_[1].value());
        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, 0);
    if(ImGui::TreeNode("CGB")) {

        ImGui::Columns(2, "cgbpalette", true);

        ImGui::Text("BGPI: %02X", ppu_->bgpi_.value()); ImGui::NextColumn();
        ImGui::Text("OBPI: %02X", ppu_->obpi_.value()); ImGui::NextColumn();
        ImGui::Separator();

        const auto draw_cgb_palettes = [&](auto name, const auto& palettes) {
            auto index = 0;
            for(const auto& palette : palettes) {
                ImGui::Text("%s%d", name, index);
                ImGui::SameLine(0, 5.f);

                for(const auto& color : palette.colors) {
                    const auto corrected = ppu_->correct_color(color);
                    const ImVec4 c{
                        corrected.red / 255.f,
                        corrected.green / 255.f,
                        corrected.blue / 255.f,
                        1.f
                    };

                    ImGui::ColorButton("", c, 0, ImVec2(20,20));
                    ImGui::SameLine(0, 10.f);
                }

                ImGui::NewLine();
                ++index;
            }
        };

        draw_cgb_palettes("BG", ppu_->cgb_bg_palettes_);
        ImGui::NextColumn();
        draw_cgb_palettes("OBJ", ppu_->cgb_obj_palettes_);
        ImGui::Columns(1);

        ImGui::TreePop();
    }

    ImGui::Spacing();
    ImGui::Separator();
}

void gameboy::ppu_debugger::draw_dma() const noexcept
{
    ImGui::Text("DMA Transfer: %s", ppu_->dma_transfer_.disabled() ? "disabled" : "enabled");
    ImGui::Separator();

    ImGui::Text("Source:           %04X", ppu_->dma_transfer_.source.value());
    ImGui::Text("Destination:      %04X", ppu_->dma_transfer_.destination.value());
    ImGui::Text("LengthModeStart:  %04X", ppu_->dma_transfer_.length_mode_start.value());

    ImGui::Spacing();

    ImGui::Text("Length:           %04X", ppu_->dma_transfer_.length());
}

void gameboy::ppu_debugger::draw_vram_view()
{
    if(ImGui::BeginTabBar("vramviewtabs")) {
        if(ImGui::BeginTabItem("BG Map")) {
            draw_bg_map();
            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("Tiles")) {
            draw_tiles();
            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("OAM")) {
            draw_oam();
            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("Palettes")) {
            draw_palettes();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

void gameboy::ppu_debugger::draw_tiles()
{
    static int current_tile_area = 0;
    if(ppu_->bus_->get_cartridge()->cgb_enabled()) {
        constexpr std::array bank_names{"Bank0", "Bank1"};
        ImGui::Combo("Tile Area", &current_tile_area, bank_names.data(), bank_names.size());
    }

    const auto ram = ppu_->ram_;

    const auto palette = ppu_->bus_->get_cartridge()->cgb_enabled()
        ? ppu_->cgb_bg_palettes_[0]
        : palette::from(ppu_->gb_palette_, ppu_->bgp_.value());

    constexpr auto tiles_physical_size = 6144u;
    constexpr auto tile_physical_size = 16u;
    auto tile_x = 0;
    auto tile_y = 0;
    for(size_t i = 0; i < tiles_physical_size; i += tile_physical_size) {
        std::array<uint8_t, tile_physical_size> tile_data;
        std::copy(
            begin(ram) + i + 8_kb * current_tile_area,
            begin(ram) + i + 8_kb * current_tile_area + tile_physical_size,
            begin(tile_data));

        for(auto row = 0; row < ppu::tile_pixel_count; ++row) {
            const auto tile_dot_lsb = tile_data[row * 2];
            const auto tile_dot_msb = tile_data[row * 2 + 1];

            for(auto col = 0; col < ppu::tile_pixel_count; ++col) {
                const auto col_lsb = (tile_dot_lsb >> (ppu::tile_pixel_count - col - 1)) & 0x1;
                const auto col_msb = (tile_dot_msb >> (ppu::tile_pixel_count - col - 1)) & 0x1;
                const auto dot = (col_msb << 1) | col_lsb;

                const auto color = ppu_->bus_->get_cartridge()->cgb_enabled()
                    ? ppu_->correct_color(palette.colors[dot])
                    : palette.colors[dot];

                tiles_img_.setPixel(
                    tile_x * ppu::tile_pixel_count + col,
                    tile_y * ppu::tile_pixel_count + row,
                    sf::Color{
                        color.red,
                        color.green,
                        color.blue,
                        255
                    });
            }
        }

        ++tile_x;
        if(tile_x == tiles_per_row) {
            tile_x = 0;
            ++tile_y;
        }
    }

    constexpr auto tiles_scale = 3.f;

    tiles_.update(tiles_img_);
    ImVec2 img_start = ImGui::GetCursorScreenPos();
    ImGui::Image(tiles_, {tiles_.getSize().x * tiles_scale, tiles_.getSize().y * tiles_scale});

    if(ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();

        const auto mouse_pos = ImGui::GetIO().MousePos;
        const int tile_y = (mouse_pos.y - img_start.y) / (tiles_scale * ppu::tile_pixel_count);
        const int tile_x = (mouse_pos.x - img_start.x) / (tiles_scale * ppu::tile_pixel_count);

        sf::Sprite zoomed_tile{tiles_, {{tile_x * 8, tile_y * 8}, {ppu::tile_pixel_count, ppu::tile_pixel_count}}};
        ImGui::Image(zoomed_tile, {128, 128});

        const auto no = tile_y * tiles_per_row + tile_x;
        ImGui::Text("no:      %02X", no & 0xFFu);
        ImGui::Text("addr:    0x%04X", 0x8000u + no * 0x10u);

        ImGui::EndTooltip();
    }
}

void gameboy::ppu_debugger::draw_bg_map()
{
    constexpr std::array bg_map_areas{"Primary(0x9800)", "Secondary(0x9C00)"};
    static int current_bg_map_area = 0;
    ImGui::Combo("BG Map Area", &current_bg_map_area, bg_map_areas.data(), bg_map_areas.size());

    constexpr std::array tile_addresses{"0x8800", "0x8000"};
    static int current_tile_address = 0;
    ImGui::Combo("Tile Address", &current_tile_address, tile_addresses.data(), tile_addresses.size());

    const auto tile_start_addr = address16(current_bg_map_area == 1 ? 0x9C00u : 0x9800u);
    for(size_t y = 0u; y < bg_map_tile_count; ++y) {
        for(size_t x = 0u; x < bg_map_tile_count; ++x) {
            const auto idx = y * bg_map_tile_count + x;
            const auto tile_no = ppu_->read_ram_by_bank(tile_start_addr + idx, 0);
            const attributes::bg tile_attr{ppu_->read_ram_by_bank(tile_start_addr + idx, 1)};

            for(auto tile_y = 0u; tile_y < ppu::tile_pixel_count; ++tile_y) {
                const auto tile_base_addr = current_tile_address == 1
                    ? ppu_->tile_address<uint8_t>(0x8000u, tile_no)
                    : ppu_->tile_address<int8_t>(0x9000u, tile_no);

                auto tile_row = ppu_->get_tile_row(tile_attr.v_flipped() ? 7u - tile_y : tile_y, tile_base_addr, tile_attr.vram_bank());

                if(tile_attr.h_flipped()) {
                    std::reverse(begin(tile_row), end(tile_row));
                }

                for(auto tile_x = 0u; tile_x < ppu::tile_pixel_count; ++tile_x) {
                    const auto color_idx = tile_row[tile_x];
                    const auto color = [&]() {
                        if(ppu_->bus_->get_cartridge()->cgb_enabled()) {
                            return ppu_->correct_color(
                                ppu_->cgb_bg_palettes_[tile_attr.palette_index()].colors[color_idx]);
                        } 

                        const auto background_palette = palette::from(ppu_->gb_palette_, ppu_->bgp_.value());
                        return background_palette.colors[color_idx];
                    }();

                    bg_map_img_.setPixel(
                        x * ppu::tile_pixel_count + tile_x,
                        y * ppu::tile_pixel_count + tile_y, sf::Color{
                            color.red,
                            color.green,
                            color.blue,
                            255
                        });
                }
            }
        }
    }

    draw_bg_map_overlay();

    bg_map_.update(bg_map_img_);

    constexpr auto bg_map_scale = 2.f;

    ImVec2 img_start = ImGui::GetCursorScreenPos();
    ImGui::Image(bg_map_, {bg_map_.getSize().x * bg_map_scale, bg_map_.getSize().y * bg_map_scale});
    if(ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();

        const auto mouse_pos = ImGui::GetIO().MousePos;
        const int tile_y = static_cast<int>((mouse_pos.y - img_start.y) / (bg_map_scale * ppu::tile_pixel_count));
        const int tile_x = static_cast<int>((mouse_pos.x - img_start.x) / (bg_map_scale * ppu::tile_pixel_count));

        sf::Sprite zoomed_tile{bg_map_, {{tile_x * 8, tile_y * 8}, {ppu::tile_pixel_count, ppu::tile_pixel_count}}};
        ImGui::Image(zoomed_tile, {128, 128});

        const auto tile_idx = tile_y * bg_map_tile_count + tile_x;
        const auto tile_no = ppu_->read_ram_by_bank(tile_start_addr + tile_idx, 0);
        const attributes::bg tile_attr{ppu_->read_ram_by_bank(tile_start_addr + tile_idx, 1)};

        ImGui::Text("y:      %02X", tile_y);
        ImGui::Text("x:      %02X", tile_x);
        ImGui::Text("no:     %02X", tile_no);
        ImGui::Text("attr:   %02X", tile_attr.attributes);
        ImGui::Separator();
        ImGui::Text("prioritized: %d", tile_attr.prioritized());
        ImGui::Text("v flipped:   %d", tile_attr.v_flipped());
        ImGui::Text("h flipped:   %d", tile_attr.h_flipped());
        ImGui::Text("vram_bank:   %d", tile_attr.vram_bank());
        ImGui::Text("palette_idx: %d", tile_attr.palette_index());

        ImGui::EndTooltip();
    }

    ImGui::NewLine();
}

void gameboy::ppu_debugger::draw_bg_map_overlay()
{
    constexpr auto bg_map_pixel_count = bg_map_tile_count * ppu::tile_pixel_count;
    const auto scy = ppu_->scy_.value();
    const auto scx = ppu_->scx_.value();

    const auto edge_y = (scy + screen_height) % bg_map_pixel_count;
    const auto edge_x = (scx + screen_width) % bg_map_pixel_count;
    for(uint8_t x = 0; x <= screen_width; ++x) {
        const auto real_x = (scx + x) % bg_map_pixel_count;
        bg_map_img_.setPixel(real_x, scy, sf::Color::White);
        bg_map_img_.setPixel(real_x, edge_y, sf::Color::White);
    }

    for(uint8_t y = 0; y < screen_height; ++y) {
        const auto real_y = (scy + y) % bg_map_pixel_count;
        bg_map_img_.setPixel(scx, real_y, sf::Color::White);
        bg_map_img_.setPixel(edge_x, real_y, sf::Color::White);
    }
}

void gameboy::ppu_debugger::draw_oam()
{
    ImGui::Spacing();
    ImGui::Spacing();

    std::array<attributes::obj, 40> objs{};
    std::memcpy(&objs, ppu_->oam_.data(), ppu_->oam_.size());

    for(auto obj_idx = 0u; obj_idx < 40u; ++obj_idx) {
        const auto& obj = objs[obj_idx];
        auto& img = oam_imgs_[obj_idx];
        auto& tex = oam_[obj_idx];

        const auto tile_y_end = ppu_->lcdc_.large_obj() ? ppu::tile_pixel_count * 2 : ppu::tile_pixel_count;
        for(auto tile_y = 0u; tile_y < tile_y_end; ++tile_y) {
            const auto tile_row = ppu_->get_tile_row(
                tile_y, 
                ppu_->tile_address<uint8_t>(0x8000u, ppu_->lcdc_.large_obj()
                     ? obj.tile_number & 0xFEu
                     : obj.tile_number),
                obj.vram_bank());

            for(auto tile_x = 0u; tile_x < ppu::tile_pixel_count; ++tile_x) {
                const auto color_idx = tile_row[tile_x];
                const auto color = [&]() {
                    if(ppu_->bus_->get_cartridge()->cgb_enabled()) {
                        return ppu_->correct_color(ppu_->cgb_obj_palettes_[obj.cgb_palette_index()].colors[color_idx]);
                    } 
                    
                    const auto background_palette = palette::from(ppu_->gb_palette_, ppu_->obp_[obj.gb_palette_index()].value());
                    return background_palette.colors[color_idx];
                }();

                img.setPixel(tile_x, tile_y, sf::Color(
                    color.red,
                    color.green,
                    color.blue,
                    color_idx == 0u ? 0u : 255u
                ));
            }
        }

        tex.update(img);

        const auto obj_disabled = obj.y == 0 || obj.y > screen_width;
        if(obj_disabled) {
            ImGui::TextColored(ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled),
              "%04X", *oam_range.begin() + obj_idx * 4u);
        } else {
            ImGui::Text("%04X", *oam_range.begin() + obj_idx * 4u);
        }
        ImGui::SameLine();

        const auto entry_color = [](const bool disabled) {
            if(disabled) {
                return sf::Color{255, 255, 255, 128};
            }

            return sf::Color::White;
        }(obj_disabled);

        if(ppu_->lcdc_.large_obj()) {
            ImGui::Image(tex, {32, 64}, entry_color);
        } else {
            ImGui::Image(tex, {64, 64},
              sf::FloatRect{0.f, 0.f, 8.f, 8.f},
              entry_color);
        }

        if(ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();

            ImGui::Text("y:          %02X", obj.y);
            ImGui::Text("x:          %02X", obj.x);
            ImGui::Text("no:         %02X", obj.tile_number);
            ImGui::Text("attr:       %02X", obj.attributes);
            ImGui::Separator();
            ImGui::Text("prioritized:  %d", obj.prioritized());
            ImGui::Text("v flipped:    %d", obj.v_flipped());
            ImGui::Text("h flipped:    %d", obj.h_flipped());
            ImGui::Text("vram_bank:    %d", obj.vram_bank());
            ImGui::Text(" gb plt_idx:  %d", obj.gb_palette_index());
            ImGui::Text("cgb plt_idx:  %d", obj.cgb_palette_index());

            ImGui::EndTooltip();
        }

        if((obj_idx + 1) % 5 != 0) {
            ImGui::SameLine(0, 32);
        } else {
            ImGui::Spacing();
            ImGui::Spacing();
        }
    }
}
