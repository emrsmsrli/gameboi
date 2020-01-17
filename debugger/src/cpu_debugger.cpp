#include <magic_enum.hpp>

#include "debugger/cpu_debugger.h"
#include "gameboy/cpu/cpu.h"
#include "gameboy/bus.h"
#include "gameboy/cartridge.h"
#include "imgui.h"

using namespace magic_enum::bitwise_operators;

gameboy::cpu_debugger::cpu_debugger(observer<cpu> cpu) noexcept
    : cpu_{cpu} {}

void gameboy::cpu_debugger::draw() const noexcept
{
    if(!ImGui::Begin("CPU")) {
        ImGui::End();
        return;
    }

    ImGui::Text("Total cycles: %lld", cpu_->total_cycles_);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    draw_registers();
    ImGui::Separator();

    ImGui::Spacing();
    ImGui::Spacing();

    draw_interrupts();

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
