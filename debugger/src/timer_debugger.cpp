#include "debugger/timer_debugger.h"
#include "gameboy/timer/timer.h"
#include "imgui.h"

gameboy::timer_debugger::timer_debugger(const observer<timer> timer) noexcept
    : timer_{timer}
{
}

void gameboy::timer_debugger::draw() const noexcept
{
    if(!ImGui::Begin("Timer")) {
        ImGui::End();
        return;
    }

    ImGui::Columns(2, "timer", true);

    ImGui::Text("Internal"); ImGui::NextColumn();
    ImGui::Text("Registers"); ImGui::NextColumn();
    ImGui::Separator();

    ImGui::Text("Timer clock:   %08llX", timer_->timer_clock_);
    ImGui::Text("Divider clock: %08llX", timer_->div_clock_);

    ImGui::NextColumn();

    ImGui::Text("DIV:  %02X", timer_->div_.value());
    ImGui::Text("TIMA: %02X", timer_->tima_.value());
    ImGui::Text("TMA:  %02X", timer_->tma_.value());
    ImGui::Text("TAC:  %02X", timer_->tac_.value());

    ImGui::Columns(1);
    ImGui::Spacing();

    ImGui::Text("Enabled: %d", timer_->timer_enabled());
    ImGui::SameLine(0, 20.f);
    ImGui::Text("Frequency: %s", [&]() {
        switch(timer_->timer_clock_freq_select()) {
            case 1024u: return "4096 Hz";
            case 16u:   return "262144 Hz";
            case 64u:   return "65536 Hz";
            case 256u:  return "16384 Hz";
            default: return "invalid";
        }
    }());

    ImGui::End();
}
