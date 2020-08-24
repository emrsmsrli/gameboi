#include "debugger/link_debugger.h"

#include "gameboy/link/link.h"
#include "imgui.h"

namespace gameboy {

void link_debugger::draw() noexcept
{
    if(!ImGui::Begin("Link")) {
        ImGui::End();
        return;
    }

    ImGui::TextColored(ImVec4{.5f, .5f, .5f, 1.f}, "SB: %02X", link_->sb_.value());
    ImGui::Separator();
    ImGui::TextColored(ImVec4{.5f, .5f, .5f, 1.f}, "SC: %02X", link_->sc_.value());
    ImGui::Separator();

    ImGui::Spacing();

    ImGui::Text("transferring   %d", link_->is_transferring());
    ImGui::Text("clock mode:    %s", [](const link::mode mode) {
        switch(mode) {
            case link::mode::external: return "external";
            case link::mode::internal: return "internal";
        }
    }(link_->clock_mode()));
    ImGui::Text("clock speed:   %s", [](const uint16_t cycles) {
        switch(cycles) {
            case 512u: return "8KHz";
            case 256u: return "16Khz";
            case  16u: return "32Khz";
            case   8u: return "64Khz";
            default: return "invalid";
        }
    }(link_->clock_rate()));

    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("shift counter: %04X", link_->shift_counter_);
    ImGui::Text("shift clock:   %02X", link_->shift_clock_);

    ImGui::End();
}

} // namespace gameboy
