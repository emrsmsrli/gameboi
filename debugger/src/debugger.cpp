#include "debugger/debugger.h"

#include <spdlog/sinks/stdout_color_sinks.h>

#include "gameboy/util/observer.h"
#include "imgui-SFML.h"

namespace gameboy {

debugger::debugger(const observer<bus> bus)
    : bus_{bus},
      apu_debugger_{bus_->get_apu()},
      cpu_debugger_{bus_->get_cpu()},
      cartridge_debugger_{bus_->get_cartridge(), make_observer(cpu_debugger_)},
      ppu_debugger_{bus_->get_ppu()},
      timer_debugger_{bus_->get_timer()},
      joypad_debugger_{bus_->get_joypad()},
      memory_bank_debugger_{bus},
      logger_{spdlog::stdout_color_st("debugger")},
      window_{
          sf::VideoMode{1600, 1200},
          "Debugger"
      }
{
    ImGui::SFML::Init(window_);
    window_.resetGLStates();
    logger_->info("debugger initialized");
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

    apu_debugger_.draw();
    cpu_debugger_.draw();
    ppu_debugger_.draw();
    timer_debugger_.draw();
    memory_bank_debugger_.draw();
    joypad_debugger_.draw();
    cartridge_debugger_.draw();

    window_.clear(sf::Color::Black);
    ImGui::SFML::Render(window_);

    window_.display();
}

void debugger::on_instruction(const address16& addr, const instruction::info& info, const uint16_t data) noexcept {
    cpu_debugger_.on_instruction(addr, info, data);
}

} // namespace gameboy
