#ifndef GAMEBOY_DEBUGGER_UTIL_H
#define GAMEBOY_DEBUGGER_UTIL_H

#include <string_view>

#include "imgui.h"

namespace gameboy {

inline void show_string_view(const std::string_view view)
{
    if(!view.empty())  {
        ImGui::TextUnformatted(view.data(), view.data() + view.size());
    } else {
        ImGui::TextUnformatted("empty_view");
    }
};

} // namespace gameboy

#endif //GAMEBOY_DEBUGGER_UTIL_H
