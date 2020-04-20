#include <magic_enum.hpp>

#include <fmt/format.h>

#include "debugger/cartridge_debugger.h"
#include "debugger/cpu_debugger.h"
#include "debugger/debugger_util.h"
#include "gameboy/cartridge.h"
#include "gameboy/cpu/cpu.h"
#include "gameboy/util/overloaded.h"
#include "imgui.h"

namespace gameboy {

cartridge_debugger::cartridge_debugger(observer<cartridge> cartridge, observer<cpu_debugger> cpu_debugger)
    : cartridge_{cartridge}, cpu_debugger_{cpu_debugger}
{
    disassemblies_.reserve(1024);
    using namespace gameboy;

    auto bank_index = 0;
    
    const auto& rom = cartridge->rom();

    const auto virtual_address = [](auto addr) -> uint16_t { return addr % 16_kb; };
    const auto make_dissassembly = [&](auto i) {
        auto [instruction_info, is_cgb] = rom[i] == 0xCB
            ? std::make_pair(instruction::extended_instruction_set[rom[i + 1]], true)
            : std::make_pair(instruction::standard_instruction_set[rom[i]], false);

        auto& diss = disassemblies_.emplace_back(
            bank_index,
            make_address(virtual_address(i)) + (bank_index < 1 ? 0 : 16_kb),
            instruction_info,
            "ROM");

        if(instruction_info.length == 0) {
            instruction_info.length = 1;
        }

        if(instruction_info.length == 1) {
            diss.representation = instruction_info.mnemonic;
        } else {
            uint16_t data = 0;
            for(auto d_i = 0; d_i < instruction_info.length - 1; ++d_i) {
                data |= rom[i + d_i + 1] << (d_i * 8);
            }

            diss.representation = fmt::format(instruction_info.mnemonic.data(), data);
        }
        
        i += instruction_info.length;
        if(is_cgb) {
            ++i;
        }

        return i;
    };

    for(size_t i = 0; i < 0x0104u;) {
        i = make_dissassembly(i);
    }
    
    for(size_t i = 0x0150u; i < rom.size();) {
        if(i % 16_kb == 0) {
            ++bank_index;
        }

        i = make_dissassembly(i);
    }
}

void cartridge_debugger::draw() noexcept
{
    if(!ImGui::Begin("Cartridge")) {
        ImGui::End();
        return;
    }

    draw_info();

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    draw_rom_disassembly();
    
    ImGui::End();
}

void cartridge_debugger::draw_info() const
{
    if(ImGui::BeginTabBar("cartinfotab")) {
        if(ImGui::BeginTabItem("Info")) {
            ImGui::Columns(2, "cartridgeinfo", false);

            ImGui::TextUnformatted("Name:");
            ImGui::TextUnformatted("CGB Flag:");
            ImGui::TextUnformatted("ROM Type:");
            ImGui::TextUnformatted("RAM Type:");

            ImGui::NextColumn();

            ImGui::TextUnformatted(cartridge_->name_.c_str());
            
            show_string_view(cartridge_->cgb_type_);
            show_string_view(cartridge_->rom_type_);
            show_string_view(cartridge_->ram_type_);

            ImGui::Columns(1);
            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("MBC")) {
            
            ImGui::TextUnformatted("type:        "); ImGui::SameLine(0, 0);
            show_string_view(cartridge_->mbc_type_);
            ImGui::Text("has battery: %d", cartridge_->has_battery());
            ImGui::Text("has rtc:     %d", cartridge_->has_rtc());
            ImGui::Text("ram enabled: %d", cartridge_->ram_enabled());
            ImGui::Text("rom banks:   %d", cartridge_->rom_bank_count());
            ImGui::Text("ram banks:   %d", cartridge_->ram_bank_count());

            if(cartridge_->has_rtc()) {
                std::visit(overloaded{
                    [](mbc1& mbc) {
                        ImGui::Text("rom banking enabled: %d", mbc.rom_banking_active());
                    },
                    [](mbc3& mbc) {
                        ImGui::Spacing();

                        ImGui::Text("rtc enabled:             %d", mbc.rtc_enabled_);
                        ImGui::Text("rtc last time:           %lld", mbc.rtc_last_time_);
                        ImGui::Text("rtc selected register:   %d", mbc.rtc_selected_register_idx_);
                        ImGui::Text("rtc latch on next write: %d", mbc.rtc_latch_data_);

                        ImGui::Spacing();
                        ImGui::Columns(2, "rtcregs", true);

                        ImGui::TextUnformatted("rtc data"); ImGui::NextColumn();
                        ImGui::TextUnformatted("rtc latch"); ImGui::NextColumn();
                        ImGui::Separator();

                        ImGui::Text("seconds %d", mbc.rtc_.seconds);
                        ImGui::Text("minutes %d", mbc.rtc_.minutes);
                        ImGui::Text("hours   %d", mbc.rtc_.hours);
                        ImGui::Text("days lo %d", mbc.rtc_.days_lower);
                        ImGui::Text("days hi %d", mbc.rtc_.days_higher);

                        ImGui::NextColumn();

                        ImGui::Text("seconds %d", mbc.rtc_latch_.seconds);
                        ImGui::Text("minutes %d", mbc.rtc_latch_.minutes);
                        ImGui::Text("hours   %d", mbc.rtc_latch_.hours);
                        ImGui::Text("days lo %d", mbc.rtc_latch_.days_lower);
                        ImGui::Text("days hi %d", mbc.rtc_latch_.days_higher);

                        ImGui::Columns(1);
                    },
                    [](auto&&) {}
                }, cartridge_->mbc_);
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

void cartridge_debugger::draw_rom_disassembly() const noexcept
{
    const auto pc_addr = make_address(cpu_debugger_->get_pc());
    const auto it = std::find_if(
        std::begin(disassemblies_), 
        std::end(disassemblies_),
        [&](const instruction::disassembly& diss) {
            return diss.address == pc_addr;
        });

    if(ImGui::BeginChild("rom_disassembly")) {
        if(it == std::end(disassemblies_)) {
            // todo parse on the fly


            ImGui::EndChild();
            return;
        }
        
        constexpr auto half_size = 5;
        const auto distance = std::distance(begin(disassemblies_), it);
        do_draw_rom_disassembly(disassemblies_, distance - half_size, distance + half_size, true);
        ImGui::EndChild();
    }
}

void cartridge_debugger::do_draw_rom_disassembly(const std::vector<instruction::disassembly>& disassemblies,
    const uint32_t start, const uint32_t end, const bool auto_scroll) const noexcept
{
    const auto clamped_start = std::max(0u, start);
    const auto clamped_end = std::min(static_cast<size_t>(end), disassemblies.size());
    const auto pc_addr = make_address(cpu_debugger_->get_pc());

    for(auto i = clamped_start; i < clamped_end; ++i) {
        const auto& data = disassemblies[i];
        const auto str = fmt::format("{}{}:{:04X} | {}", 
            data.area, data.bank, data.address.value(), data.representation);
        
        ImGui::PushStyleColor(ImGuiCol_Text, [&]() {
            if(data.address == pc_addr) {
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
        ImGui::TextUnformatted(str.c_str());
        ImGui::PopStyleColor();
        
        if(pc_addr == data.address && auto_scroll) {
            ImGui::SetScrollHereY();
        }
    }
}

} // namespace gameboy
