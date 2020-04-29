#include "debugger/cartridge_debugger.h"

#include "debugger/cpu_debugger.h"
#include "debugger/debugger_util.h"
#include "gameboy/cartridge.h"
#include "gameboy/cpu/cpu.h"
#include "gameboy/util/overloaded.h"
#include "imgui.h"

namespace gameboy {

cartridge_debugger::cartridge_debugger(observer<cartridge> cartridge)
    : cartridge_{cartridge} {}

void cartridge_debugger::draw() noexcept
{
    if(!ImGui::Begin("Cartridge")) {
        ImGui::End();
        return;
    }

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
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::TextUnformatted("MBC"); ImGui::Separator();
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

    ImGui::End();
}

} // namespace gameboy
