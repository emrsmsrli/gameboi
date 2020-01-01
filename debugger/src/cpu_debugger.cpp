#include <array>

#include "debugger/cpu_debugger.h"
#include "gameboy/cpu/cpu.h"
#include "gameboy/bus.h"
#include "gameboy/cartridge.h"
#include "imgui.h"

namespace {

struct instruction {

};

constexpr std::array instructions {
    instruction{}
};

}

gameboy::cpu_debugger::cpu_debugger(observer<cpu> cpu) noexcept
    : cpu_{cpu}
{
    const auto& rom = cpu_->bus_->get_cartridge()->rom();
    for(size_t i = 0; i < 0x0104u; ++i) {
        
    }
    for(size_t i = 0x0150u; i < rom.size(); ++i) {
        
    }
    // todo decode rom
}

void gameboy::cpu_debugger::draw() const noexcept
{
    if(!ImGui::Begin("CPU")) {
        ImGui::End();
        return;
    }
    
    draw_registers();
    ImGui::Separator();

    ImGui::Spacing();
    ImGui::Spacing();

    draw_interrupts();
    ImGui::Separator();

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    // todo decode instructions

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
