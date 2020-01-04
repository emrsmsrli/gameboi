#include "debugger/memory_bank_debugger.h"
#include "gameboy/bus.h"
#include "gameboy/cartridge.h"
#include "gameboy/memory/mmu.h"
#include "gameboy/ppu/ppu.h"
#include "imgui.h"

gameboy::memory_bank_debugger::memory_bank_debugger(const observer<bus> bus) noexcept
    : bus_{bus}
{
    memory_editor_.ReadOnly = true;
}

void gameboy::memory_bank_debugger::draw() noexcept
{
    if(!ImGui::Begin("Memory")) {
        ImGui::End();
        return;
    }

    if(ImGui::BeginTabBar("memorytabs")) {
        if(ImGui::BeginTabItem("Cartridge")) {
            if(ImGui::BeginTabBar("cartridgememorytabs")) {
                auto cartridge = bus_->get_cartridge();
                if(ImGui::BeginTabItem("ROM")) {
                    memory_editor_.DrawContents(cartridge->rom_.data(), cartridge->rom_.size());
                    ImGui::EndTabItem();
                }

                if(!cartridge->ram_.empty()) {
                    if(ImGui::BeginTabItem("RAM")) {
                        memory_editor_.DrawContents(cartridge->ram_.data() + cartridge->ram_bank() * 8_kb, 
                            8_kb, 0xA000);
                        ImGui::EndTabItem();
                    }
                }

                ImGui::EndTabBar();
            }

            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("Internal")) {
            if(ImGui::BeginTabBar("internalmemorytabs")) {
                auto mmu = bus_->get_mmu();
                if(ImGui::BeginTabItem("WRAM")) {
                    ImGui::Text("SVBK: %02X", mmu->wram_bank_);
                    ImGui::Spacing();

                    memory_editor_.DrawContents(mmu->work_ram_.data() + mmu->wram_bank_ * 4_kb, 
                        4_kb, 0xC000);
                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem("HRAM")) {
                    memory_editor_.DrawContents(mmu->high_ram_.data(), mmu->high_ram_.size(), 0xFF80);
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("PPU")) {
            if(ImGui::BeginTabBar("ppumemorytabs")) {
                auto ppu = bus_->get_ppu();
                if(ImGui::BeginTabItem("VRAM")) {
                    ImGui::Text("VBK: %02X", ppu->vram_bank_);
                    ImGui::Spacing();
                    
                    memory_editor_.DrawContents(ppu->ram_.data() + ppu->vram_bank_ * 8_kb, 
                        8_kb, 0x8000);
                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem("OAM")) {                    
                    memory_editor_.DrawContents(ppu->oam_.data(), ppu->oam_.size(), 0xFE00);
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
