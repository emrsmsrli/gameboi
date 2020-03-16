#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>

#include "gameboy/gameboy.h"
#include "gameboy/version.h"

namespace gameboy {

gameboy::gameboy(const std::string_view rom_path)
    : cartridge_{rom_path},
      bus_{make_observer(this)},
      mmu_{make_observer(bus_)},
      cpu_{make_observer(bus_)},
      ppu_{make_observer(bus_)},
      apu_{make_observer(bus_)},
      link_{make_observer(bus_)},
      joypad_{make_observer(bus_)},
      timer_{make_observer(bus_)}
{
    spdlog::info("gameboy v{}", version::version);
}

void gameboy::start()
{
    using namespace std::chrono;

    constexpr auto delay = 16.742ms;
    auto next_tick = steady_clock::now() + delay;

    while(true) {
        tick_one_frame();

        std::this_thread::sleep_until(next_tick);
        next_tick += delay;
        break;
    }
}

void gameboy::tick()
{
    const auto cycles = cpu_.tick();

    if(!cpu_.is_stopped()) {
        ppu_.tick(cycles);
        apu_.tick(cycles);
        link_.tick(cycles);
        timer_.tick(cycles);
    }

    cpu_.process_interrupts();
}

void gameboy::tick_one_frame()
{
    while(mmu_.read(ppu::ly_addr) != 0x00u) {
        tick();
    }

    while(mmu_.read(ppu::ly_addr) < 144) {
        tick();
    }
}

} // namespace gameboy
