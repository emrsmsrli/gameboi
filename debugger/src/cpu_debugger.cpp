#include "debugger/cpu_debugger.h"
#include "gameboy/cpu/cpu.h"
#include "imgui.h"

gameboy::cpu_debugger::cpu_debugger(observer<cpu> cpu) noexcept
    : cpu_{cpu} {}

void gameboy::cpu_debugger::draw() const noexcept
{
    if(!ImGui::Begin("CPU")) {
        ImGui::End();
        return;
    }

    draw_registers();
    draw_interrupts();

    // todo decode instructions

    ImGui::End();

    ImGui::ShowDemoWindow();
}

void gameboy::cpu_debugger::draw_registers() const noexcept
{
    ImGui::Columns(3, "Registers And Flags", true);

    ImGui::Text("%s: %04X", "AF", cpu_->a_f_.high().value());
    ImGui::Text("%s: %04X", "BC", cpu_->b_c_.high().value());
    ImGui::Text("%s: %04X", "DE", cpu_->d_e_.high().value());
    ImGui::Text("%s: %04X", "HL", cpu_->h_l_.high().value());

    ImGui::NextColumn();

    ImGui::Text("SP: %04X", cpu_->stack_pointer_.value());
    ImGui::Text("PC: %04X", cpu_->program_counter_.value());
    
    ImGui::NextColumn();

    ImGui::Text("Z: %d", cpu_->test_flag(cpu::flag::zero));
    ImGui::Text("N: %d", cpu_->test_flag(cpu::flag::subtract));
    ImGui::Text("H: %d", cpu_->test_flag(cpu::flag::half_carry));
    ImGui::Text("C: %d", cpu_->test_flag(cpu::flag::carry));

    ImGui::Columns(1);
}

void gameboy::cpu_debugger::draw_interrupts() const noexcept
{
    
}
