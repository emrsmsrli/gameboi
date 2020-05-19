#include "debugger/debugger.h"

#include <spdlog/sinks/stdout_color_sinks.h>

#include "gameboy/gameboy.h"
#include "imgui-SFML.h"

namespace gameboy {

debugger::debugger(const observer<gameboy> gb)
    : gb_{gb},
      bus_{gb_->get_bus()},
      apu_debugger_{bus_->get_apu()},
      cpu_debugger_{bus_->get_cpu()},
      cartridge_debugger_{bus_->get_cartridge()},
      ppu_debugger_{bus_->get_ppu()},
      timer_debugger_{bus_->get_timer()},
      joypad_debugger_{bus_->get_joypad()},
      disassembly_view_{bus_, make_observer(cpu_debugger_)},
      memory_bank_debugger_{bus_},
      logger_{spdlog::stdout_color_st("debugger")},
      window_{
          sf::VideoMode{1600, 1200},
          "Debugger"
      }
{
    bus_->get_cpu()->on_instruction({connect_arg<&debugger::on_instruction>, this});
    bus_->get_mmu()->on_read_access({connect_arg<&debugger::on_read_access>, this});
    bus_->get_mmu()->on_write_access({connect_arg<&debugger::on_write_access>, this});

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
    disassembly_view_.draw();

    window_.clear(sf::Color::Black);
    ImGui::SFML::Render(window_);

    window_.display();
}

void debugger::on_instruction(const address16& addr, const instruction::info& info, const uint16_t data) noexcept
{
    cpu_debugger_.on_instruction(addr, info, data);
    if(has_execution_breakpoint()) {
        gb_->tick_enabled = false;
    }
}

void debugger::on_write_access(const address16& addr, uint8_t data) noexcept
{
    disassembly_view_.on_write_access(addr, data);
    if(has_write_access_breakpoint(addr, data)) {
        gb_->tick_enabled = false;
    }
}

void debugger::on_read_access(const address16& addr) noexcept
{
    if(has_read_access_breakpoint(addr)) {
        gb_->tick_enabled = false;
    }
}

} // namespace gameboy
