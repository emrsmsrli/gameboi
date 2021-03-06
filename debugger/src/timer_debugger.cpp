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

    ImGui::TextUnformatted("Internal clock:");
    ImGui::Text("%04X", timer_->internal_clock_);

    ImGui::NextColumn();

    ImGui::Text("DIV:  %02X", timer_->internal_clock_ >> 8u);
    ImGui::Text("TIMA: %02X", timer_->tima_.value());
    ImGui::Text("TMA:  %02X", timer_->tma_.value());
    ImGui::Text("TAC:  %02X", timer_->tac_.value());

    ImGui::Columns(1);
    ImGui::Spacing();

    ImGui::Text("Enabled: %d", static_cast<int>(timer_->enabled_));
    ImGui::SameLine(0, 20.f);
    ImGui::Text("Frequency: %s", [&]() {
        switch(timer_->timer_clock_overflow_bit_) {
            case 9u: return "4 KHz";
            case 3u: return "256 KHz";
            case 5u: return "64 KHz";
            case 7u: return "16 KHz";
            default: return "invalid";
        }
    }());

    ImGui::End();
}
