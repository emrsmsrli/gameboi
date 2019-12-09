#include "debugger/debugger.h"
#include "imgui-SFML.h"
#include "imgui.h"

namespace gameboy {

debugger::debugger(const observer<const bus> bus)
    : bus_{bus},
      window_{
          sf::VideoMode{1200, 1200},
          "Debugger"
      }
{
    ImGui::SFML::Init(window_);
    window_.resetGLStates();
}

debugger::~debugger()
{
    ImGui::SFML::Shutdown();
}

void debugger::tick()
{
    sf::Event event{};
    while(window_.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);
    }

    ImGui::SFML::Update(window_, delta_clock_.restart());

    // todo draw ui
    ImGui::Begin("Hello, world!");
    ImGui::Button("Look at this pretty button");
    ImGui::End();

    window_.clear(sf::Color::Black);
    ImGui::SFML::Render(window_);

    window_.display();
}

} // namespace gameboy
