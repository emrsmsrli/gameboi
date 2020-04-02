#include "debugger/debugger.h"

#include "gameboy/util/observer.h"

#include "imgui-SFML.h"

namespace gameboy {

debugger::debugger(const observer<bus> bus)
    : gb_tick_allowed{false},
      bus_{bus},
      cartridge_debugger_{bus_->get_cartridge(), bus_->get_cpu()},
      cpu_debugger_{bus_->get_cpu(), make_observer(cartridge_debugger_)},
      ppu_debugger_{bus_->get_ppu()},
      timer_debugger_{bus_->get_timer()},
      memory_bank_debugger_{bus},
      window_{
          sf::VideoMode{1200, 1200},
          "Debugger"
      }
{
    ImGui::SFML::Init(window_);
    window_.resetGLStates();

    cartridge_debugger_.on_break({connect_arg<&debugger::on_execution_break>, this});
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

    cpu_debugger_.draw();
    ppu_debugger_.draw();
    timer_debugger_.draw();
    memory_bank_debugger_.draw();
    cartridge_debugger_.draw();

    window_.clear(sf::Color::Black);
    ImGui::SFML::Render(window_);

    window_.display();
}

} // namespace gameboy
