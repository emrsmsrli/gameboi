#include "debugger/ppu_debugger.h"
#include "gameboy/ppu/ppu.h"
#include "gameboy/bus.h"
#include "gameboy/cartridge.h"
#include "imgui.h"

gameboy::ppu_debugger::ppu_debugger(const observer<ppu> ppu) noexcept
    : ppu_{ppu}
{
    
}

void gameboy::ppu_debugger::draw() const noexcept
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

            //draw_dma();

            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("Palettes")) {
            draw_palettes();

            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("VRAM View")) {
            // draw_vram_view();

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();

    // -- dma
    // source16
    // dest16
    // length_mode_start8
    // remaining_length_uint
    //
    // bgmap
    //
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
    ImGui::Text("WIN:  %04X", ppu_->lcdc_.window_map_address().value());
    ImGui::Text("WIN:  %s", ppu_->lcdc_.window_enabled() ? "on" : "off");
    ImGui::Text("CHR:  %04X", ppu_->lcdc_.tile_base_address().value());
    ImGui::Text("BG:   %04X", ppu_->lcdc_.bg_map_address().value());
    ImGui::Text("OBJ:  %s", ppu_->lcdc_.large_obj() ? "8x16" : "8x8");
    ImGui::Text("OBJ:  %s" ,ppu_->lcdc_.obj_enabled() ? "on" : "off");
    ImGui::Text("BG:   %s", ppu_->lcdc_.bg_enabled() ? "on" : "off");

    ImGui::NextColumn();

    const auto mode_str = [&]() {
        switch(ppu_->stat_.get_mode()) {
            case stat_mode::h_blank: return "hblank";
            case stat_mode::v_blank: return "vblank";
            case stat_mode::reading_oam: return "reading oam";
            case stat_mode::reading_oam_vram: return "reading oam vram";
        }
    }();

    using namespace gameboy;

    ImGui::Text("int LYC=LY:  %d", bit_test(ppu_->stat_.reg, 6));
    ImGui::Text("int OAM:     %d", bit_test(ppu_->stat_.reg, 5));
    ImGui::Text("int VBlank:  %d", bit_test(ppu_->stat_.reg, 4));
    ImGui::Text("int HBlank:  %d", bit_test(ppu_->stat_.reg, 3));
    ImGui::Text("LYC=LY:      %d", bit_test(ppu_->stat_.reg, 2));
    ImGui::Text("mode:        %d (%s)", static_cast<int8_t>(ppu_->stat_.get_mode()), mode_str);
    
    ImGui::Columns(1);
}

void gameboy::ppu_debugger::draw_palettes() const
{
    //
    // -- gb
    // bgp_;
    // obp_[2];
    //
    // -- cgb
    // cgb_bg_palettes[8];
    // cgb_obj_palettes[8];
    // bgpi_;
    // obpi_;
    //
    
    ImGui::SetNextItemOpen(true, 0);
    if(ImGui::TreeNode("GB")) {
        const auto draw_gb_palette = [&](auto name, auto data) {
            ImGui::SetNextItemOpen(true, 0);
            if(ImGui::TreeNode(name, "%s: %02X", name, data)) {
                for(const auto& color : palette::from(ppu::gb_palette_, data).colors) {
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

        const auto draw_cgb_palettes = [](auto name, const auto& palettes) {
            auto index = 0;
            for(const auto& palette : palettes) {
                ImGui::Text("%s%d", name, index);
                ImGui::SameLine(0, 5.f);

                for(const auto& color : palette.colors) {
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
