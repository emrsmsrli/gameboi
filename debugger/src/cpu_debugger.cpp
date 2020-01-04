#include <array>

#include <fmt/format.h>

#include "debugger/cpu_debugger.h"
#include "gameboy/cpu/cpu.h"
#include "gameboy/bus.h"
#include "gameboy/cartridge.h"
#include "imgui.h"

namespace {

}

gameboy::cpu_debugger::cpu_debugger(observer<cpu> cpu) noexcept
    : cpu_{cpu}
{
    disassemblies_.reserve(1024);
    using namespace gameboy;

    auto bank_index = 0;
    auto instruction_index = 0;
    
    const auto& rom = cpu_->bus_->get_cartridge()->rom();

    const auto virtual_address = [](auto addr) -> uint16_t { return addr % 16_kb; };
    const auto make_dissassembly = [&](auto i) {
        auto instruction_info = rom[i] == 0xCB
            ? instruction::extended_instruction_set[rom[++i]]
            : instruction::standard_instruction_set[rom[i]];

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

        i += instruction_info.length;
        ++instruction_index;
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

void gameboy::cpu_debugger::draw() const noexcept
{
    if(!ImGui::Begin("CPU")) {
        ImGui::End();
        return;
    }

    if(ImGui::BeginTabBar("cpuinfo")) {
        if(ImGui::BeginTabItem("Info")) {
            draw_registers();
            ImGui::Separator();

            ImGui::Spacing();
            ImGui::Spacing();

            draw_interrupts();

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

    ImGui::ShowDemoWindow();
}

void gameboy::cpu_debugger::draw_registers() const noexcept
{
    ImGui::Columns(3, "registers and flags", true);
    
    ImGui::Text("Registers"); ImGui::NextColumn();
    ImGui::Text("SP and PC"); ImGui::NextColumn();
    ImGui::Text("Flags"); ImGui::NextColumn();
    ImGui::Separator();

    ImGui::Text("%s: %04X", "AF", cpu_->a_f_.value());
    ImGui::Text("%s: %04X", "BC", cpu_->b_c_.value());
    ImGui::Text("%s: %04X", "DE", cpu_->d_e_.value());
    ImGui::Text("%s: %04X", "HL", cpu_->h_l_.value());

    ImGui::NextColumn();

    ImGui::Text("SP: %04X", cpu_->stack_pointer_.value());
    ImGui::Text("PC: %04X", cpu_->program_counter_.value());
    
    ImGui::NextColumn();

    ImGui::Text("Z: %d", cpu_->test_flag(cpu::flag::zero));
    ImGui::Text("N: %d", cpu_->test_flag(cpu::flag::negative));
    ImGui::Text("H: %d", cpu_->test_flag(cpu::flag::half_carry));
    ImGui::Text("C: %d", cpu_->test_flag(cpu::flag::carry));

    ImGui::Columns(1);
}

void gameboy::cpu_debugger::draw_interrupts() const noexcept
{
    ImGui::Text("ime: %s", 
        cpu_->interrupts_enabled() 
            ? "enabled" 
            : "disabled");
    
    ImGui::Columns(2, "interrupts", true);
    
    ImGui::Text("interrupt enable"); ImGui::NextColumn();
    ImGui::Text("interrupt flags");  ImGui::NextColumn();
    ImGui::Separator();

    const auto interrupt_enable = [&](auto interrupt) { return (cpu_->interrupt_enable_ & interrupt) == interrupt; };
    ImGui::Text("lcd_vblank: %d", interrupt_enable(interrupt::lcd_vblank));
    ImGui::Text("lcd_stat:   %d", interrupt_enable(interrupt::lcd_stat));
    ImGui::Text("timer:      %d", interrupt_enable(interrupt::timer));
    ImGui::Text("serial:     %d", interrupt_enable(interrupt::serial));
    ImGui::Text("joypad:     %d", interrupt_enable(interrupt::joypad));

    ImGui::NextColumn();
    
    const auto interrupt_flag = [&](auto interrupt) { return (cpu_->interrupt_flags_ & interrupt) == interrupt; };
    ImGui::Text("lcd_vblank: %d", interrupt_flag(interrupt::lcd_vblank));
    ImGui::Text("lcd_stat:   %d", interrupt_flag(interrupt::lcd_stat));
    ImGui::Text("timer:      %d", interrupt_flag(interrupt::timer));
    ImGui::Text("serial:     %d", interrupt_flag(interrupt::serial));
    ImGui::Text("joypad:     %d", interrupt_flag(interrupt::joypad));

    ImGui::Columns(1);
}

void gameboy::cpu_debugger::draw_rom_disassembly() const noexcept
{
    if(ImGui::BeginChild("rom_disassembly")) {
        const auto pc_addr = make_address(cpu_->program_counter_); 
        const auto& disassembly = *std::find_if(
            std::begin(disassemblies_), 
            std::end(disassemblies_),
            [&](const instruction_disassembly& diss) {
                return diss.address == pc_addr;
            });
        
        constexpr auto half_size = 10;
        do_draw_rom_disassembly(disassembly.index - half_size, disassembly.index + half_size);
        ImGui::EndChild();
    }
}

void gameboy::cpu_debugger::draw_rom_disassembly_full() const noexcept
{
    if(ImGui::BeginChild("rom_disassembly_full")) {
        ImGuiListClipper clipper(disassemblies_.size());
        while(clipper.Step()) {
            do_draw_rom_disassembly(clipper.DisplayStart, clipper.DisplayEnd);
        }

        ImGui::EndChild();
    }
}

void gameboy::cpu_debugger::do_draw_rom_disassembly(const uint32_t start, const uint32_t end) const noexcept
{
    const auto pc_addr = make_address(cpu_->program_counter_);
    for(auto i = start; i < end; ++i) {
        const auto& data = disassemblies_[i];
        const auto str = fmt::format("ROM{}:{:04X} | {}", 
            data.bank, data.address.value(), data.disassembly);

        if(pc_addr == data.address) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.f, 1.f, 0.f, 1.f});
            ImGui::TextUnformatted(str.c_str());
            ImGui::PopStyleColor();

            ImGui::SetScrollHereY();
        } else {
            ImGui::TextUnformatted(str.c_str());
        }
    }
}
