#include "debugger/joypad_debugger.h"

#include <magic_enum.hpp>

#include "imgui.h"

namespace gameboy {

void joypad_debugger::draw() noexcept
{
    if(!ImGui::Begin("Joypad")) {
        ImGui::End();
    }

    ImGui::Columns(2);

    ImGui::Text("joyp : %02X", joypad_->joyp_.value());
    ImGui::Separator();

    using namespace magic_enum::bitwise_operators;

    ImGui::Text("right:  %d", (joypad_->keys_ & joypad::key::right) != joypad::key::right);
    ImGui::Text("left:   %d", (joypad_->keys_ & joypad::key::left) != joypad::key::left);
    ImGui::Text("up:     %d", (joypad_->keys_ & joypad::key::up) != joypad::key::up);
    ImGui::Text("down:   %d", (joypad_->keys_ & joypad::key::down) != joypad::key::down);

    ImGui::NextColumn();

    ImGui::Text("a:      %d", (joypad_->keys_ & joypad::key::a) != joypad::key::a);
    ImGui::Text("b:      %d", (joypad_->keys_ & joypad::key::b) != joypad::key::b);
    ImGui::Text("select: %d", (joypad_->keys_ & joypad::key::select) != joypad::key::select);
    ImGui::Text("start:  %d", (joypad_->keys_ & joypad::key::start) != joypad::key::start);

    ImGui::Columns(1);
    ImGui::End();
}

} // namespace gameboy
