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
      link_debugger_{bus_->get_link()},
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

    window_.resetGLStates();
    logger_->info("debugger initialized");
}

void debugger::tick()
{
    sf::Event event{};
    while(window_.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);
    }

    const auto delta = delta_clock_.restart();
    const auto delta_secs = delta.asSeconds();
    ImGui::SFML::Update(window_, delta);

    last_frame_times_[last_frame_time_idx_] = delta_secs;
    if(++last_frame_time_idx_ == 100u) {
        last_frame_time_idx_ = 0u;
    }

    if(ImGui::Begin("Performance")) {
        ImGui::Text("FPS          %.3f", 1.f / delta_secs);
        ImGui::Text("Frame time   %.3f", delta_secs);

        ImGui::PlotLines("",
            last_frame_times_.data(),
            last_frame_times_.size(),
            last_frame_time_idx_, "",
            0.f, .1f,
            ImVec2(250.f, 100.f));

        ImGui::End();
    }

    apu_debugger_.draw();
    cpu_debugger_.draw();
    ppu_debugger_.draw();
    timer_debugger_.draw();
    memory_bank_debugger_.draw();
    joypad_debugger_.draw();
    link_debugger_.draw();
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
