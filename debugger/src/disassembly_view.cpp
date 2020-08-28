#include "debugger/disassembly_view.h"

#include <vector>

#include "debugger/cpu_debugger.h"
#include "gameboy/bus.h"
#include "gameboy/cartridge.h"
#include "gameboy/memory/memory_constants.h"
#include "gameboy/memory/mmu.h"
#include "imgui.h"

namespace gameboy {

disassembly_view::disassembly_view(const observer<bus> bus, const observer<cpu_debugger> cpu_debugger)
    : bus_{bus},
      cpu_debugger_{cpu_debugger},
      rom_db_{bus_, instruction::disassembly_db::name_rom, bus_->get_cartridge()->rom()},
      wram_db_{bus_, instruction::disassembly_db::name_wram, bus_->get_mmu()->work_ram_},
      hram_db_{bus_, instruction::disassembly_db::name_hram, bus_->get_mmu()->high_ram_} {}

void disassembly_view::draw() noexcept
{
    if(!ImGui::Begin("Disassembly view") || rom_db_.get().empty()) {
        ImGui::End();
        return;
    }

    const auto draw_all_disassemblies = [&](const auto& diss) {
        const auto pc = make_address(cpu_debugger_->get_pc());

        if(ImGui::BeginChild("all_disassemblies")) {
            ImGuiListClipper clipper(diss.size());
            while(clipper.Step()) {
                for(auto i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                    ImGui::TextUnformatted(diss[i].representation.c_str());
                }
            }

            ImGui::EndChild();
        }
    };

    if(ImGui::BeginTabBar("disassembly_view")) {
        if(ImGui::BeginTabItem("Program Counter")) {
            const auto pc = make_address(cpu_debugger_->get_pc());
            const auto& disassemblies = [&]() -> const std::vector<instruction::disassembly>& {
                if(rom_range.has(pc)) { return rom_db_.get(); }
                if(wram_range.has(pc)) { return wram_db_.get(); }
                return hram_db_.get();
            }();

            const auto it = std::find_if(
                cbegin(disassemblies),
                cend(disassemblies),
                [&](const instruction::disassembly& diss) {
                  return diss.address == pc;
                });

            constexpr auto half_size = 10;
            const auto distance = std::distance(cbegin(disassemblies), it);

            const std::ptrdiff_t start = distance - half_size;
            const auto clamped_start = std::max<std::ptrdiff_t>(0, start);
            const auto clamped_end = std::min<std::ptrdiff_t>(
                start < 0
                    ? distance + half_size - start
                    : distance + half_size,
                disassemblies.size()
            );

            for(auto i = clamped_start; i < clamped_end; ++i) {
                const auto& data = disassemblies[i];

                ImGui::PushStyleColor(ImGuiCol_Text, [&]() {
                    if(data.info.mnemonic == "invalid") {
                        return ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_TextDisabled));
                    }

                    if(data.address == pc) {
                        return ImVec4{1.f, 1.f, 0.f, 1.f};
                    }

                    const auto bb = cpu_debugger::execution_breakpoint{data.address, static_cast<int>(data.bank)};
                    auto any_bb = bb;
                    any_bb.bank = cpu_debugger::execution_breakpoint::any_bank;

                    if(cpu_debugger_->has_execution_breakpoint(bb) || cpu_debugger_->has_execution_breakpoint(any_bb)) {
                        return ImVec4{1.f, 0.f, 0.f, 1.f};
                    }

                    return ImVec4{1.f, 1.f, 1.f, 1.f};
                }());
                ImGui::TextUnformatted(data.representation.c_str());
                ImGui::PopStyleColor();
            }

            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("ROM")) {
            draw_all_disassemblies(rom_db_.get());
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("WRA")) {
            draw_all_disassemblies(wram_db_.get());
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("HRA")) {
            draw_all_disassemblies(hram_db_.get());
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void disassembly_view::on_write_access(const address16& addr, const uint8_t data) noexcept
{
    if(wram_range.has(addr)) {
        wram_db_.on_write(addr, data);
    } else if(hram_range.has(addr)) {
        hram_db_.on_write(addr, data);
    }
}

void disassembly_view::on_new_rom() noexcept
{
    rom_db_ = instruction::disassembly_db{bus_, instruction::disassembly_db::name_rom, bus_->get_cartridge()->rom()};
    wram_db_ = instruction::disassembly_db{bus_, instruction::disassembly_db::name_wram, bus_->get_mmu()->work_ram_};
    hram_db_ = instruction::disassembly_db{bus_, instruction::disassembly_db::name_hram, bus_->get_mmu()->high_ram_};
}

} // namespace gameboy
