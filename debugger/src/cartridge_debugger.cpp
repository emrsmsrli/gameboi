#include <fmt/format.h>

#include "debugger/cartridge_debugger.h"
#include "gameboy/cartridge.h"
#include "gameboy/cpu/cpu.h"
#include "debugger/debugger.h"

namespace gameboy {

cartridge_debugger::cartridge_debugger(observer<cartridge> cartridge, observer<cpu> cpu)
    : cartridge_{cartridge}, cpu_{cpu}
{
    disassemblies_.reserve(1024);
    using namespace gameboy;

    auto bank_index = 0;
    auto instruction_index = 0;
    
    const auto& rom = cartridge->rom();

    const auto virtual_address = [](auto addr) -> uint16_t { return addr % 16_kb; };
    const auto make_dissassembly = [&](auto i) {
        auto& [instruction_info, is_cgb] = rom[i] == 0xCB
            ? std::make_pair(instruction::extended_instruction_set[rom[i + 1]], true)
            : std::make_pair(instruction::standard_instruction_set[rom[i]], false);

        auto& diss = disassemblies_.emplace_back(instruction_index, bank_index, 
            make_address(virtual_address(i)) + (bank_index < 1 ? 0 : 16_kb), instruction_info);

        if(instruction_info.length == 0) {
            instruction_info.length = 1;
        }

        if(instruction_info.length == 1) {
            diss.disassembly = instruction_info.mnemonic;
        } else {
            uint16_t data = 0;
            for(auto d_i = 0; d_i < instruction_info.length - 1; ++d_i) {
                data |= rom[i + d_i + 1] << (d_i * 8);
            }

            diss.disassembly = fmt::format(instruction_info.mnemonic.data(), data);
        }

        ++instruction_index;
        
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

    if(ImGui::BeginTabBar("cartridgetabs")) {
        if(ImGui::BeginTabItem("Info")) {
            draw_info();

            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::TextUnformatted("Breakpoints");
            ImGui::Separator();
            ImGui::Spacing();

            if(std::array<char, 5> buf{}; ImGui::InputText("", buf.data(), buf.size(), 
                    ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
                const auto addr_l = std::strtoul(buf.data(), nullptr, 16);
                breakpoints_.emplace_back(addr_l);
            }

            if(!breakpoints_.empty()) {
                ImGuiListClipper clipper(breakpoints_.size());
                while(clipper.Step()) {
                    for(auto i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                        const auto deleted = ImGui::SmallButton("X");

                        ImGui::SameLine(0, 20);
                        ImGui::Text("%04X", breakpoints_[i].value());

                        if(deleted) {
                            breakpoints_.erase(begin(breakpoints_) + i);
                        }
                    }
                }
            } else {
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::TextUnformatted("No breakpoints");
            }

            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("Disassembly")) {
            draw_rom_disassembly();
            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("Full Disassembly")) {
            draw_rom_disassembly_full();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    
    ImGui::End();
}

void cartridge_debugger::check_breakpoints()
{
    if(const auto pc = make_address(cpu_->program_counter_);
        std::find(begin(breakpoints_), end(breakpoints_), pc) != end(breakpoints_)) {
        on_break_(); // break the execution
    }
}

void cartridge_debugger::draw_info() const
{
    ImGui::Columns(2, "cartridgeinfo", false);

    ImGui::TextUnformatted("Name:");
    ImGui::TextUnformatted("CGB Flag:");
    ImGui::TextUnformatted("MBC Type:");
    ImGui::TextUnformatted("ROM Type:");
    ImGui::TextUnformatted("RAM Type:");

    ImGui::NextColumn();

    ImGui::TextUnformatted(cartridge_->name_.c_str());

    const auto show_type = [](const auto& type) { ImGui::TextUnformatted(type.data(), type.data() + type.size()); };
    
    show_type(cartridge_->cgb_type_);
    show_type(cartridge_->mbc_type_);
    show_type(cartridge_->rom_type_);
    show_type(cartridge_->ram_type_);

    ImGui::Columns(1);
}

void cartridge_debugger::draw_rom_disassembly() const noexcept
{
    const auto pc_addr = make_address(cpu_->program_counter_); 
    const auto it = std::find_if(
        std::begin(disassemblies_), 
        std::end(disassemblies_),
        [&](const instruction_disassembly& diss) {
            return diss.address == pc_addr;
        });

    if(it == std::end(disassemblies_)) {
        return;
    }

    if(ImGui::BeginChild("rom_disassembly")) {
        const auto& disassembly = *it;
        
        constexpr auto half_size = 10;
        do_draw_rom_disassembly(disassembly.index - half_size, disassembly.index + half_size, true);
        ImGui::EndChild();
    }
}

void cartridge_debugger::draw_rom_disassembly_full() const noexcept
{
    if(ImGui::BeginChild("rom_disassembly_full")) {
        ImGuiListClipper clipper(disassemblies_.size());
        while(clipper.Step()) {
            do_draw_rom_disassembly(clipper.DisplayStart, clipper.DisplayEnd, false);
        }

        ImGui::EndChild();
    }
}

void cartridge_debugger::do_draw_rom_disassembly(const uint32_t start, const uint32_t end, const bool auto_scroll) const noexcept
{
    const auto clamped_start = std::max(0u, start);
    const auto clamped_end = std::min(static_cast<size_t>(end), disassemblies_.size());
    const auto pc_addr = make_address(cpu_->program_counter_);
    for(auto i = clamped_start; i < clamped_end; ++i) {
        const auto& data = disassemblies_[i];
        const auto str = fmt::format("ROM{}:{:04X} | {}", 
            data.bank, data.address.value(), data.disassembly);

        if(pc_addr == data.address) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.f, 1.f, 0.f, 1.f});
            ImGui::TextUnformatted(str.c_str());
            ImGui::PopStyleColor();

            if(auto_scroll) {
                ImGui::SetScrollHereY();
            }
        } else {
            ImGui::TextUnformatted(str.c_str());
        }
    }
}

} // namespace gameboy
