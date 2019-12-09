#include <chrono>
#include <thread>

#include "gameboy/gameboy.h"

#include "version.h"

namespace gameboy {

gameboy::gameboy(const std::string_view rom_path)
    : cartridge_{rom_path},
      bus_{make_observer(this)},
      mmu_{make_observer(bus_)},
      cpu_{make_observer(bus_)},
      ppu_{make_observer(bus_)},
      apu_{make_observer(bus_)},
      joypad_{make_observer(bus_)}
{
    log::info("gameboy v{}", version::version);
}

void gameboy::start()
{
    using namespace std::chrono;

    constexpr auto delay = 16.742ms;
    auto next_tick = steady_clock::now() + delay;

    while(true) {
        tick();

        std::this_thread::sleep_until(next_tick);
        next_tick += delay;
        break;
    }
}
void gameboy::tick()
{
    const auto cycles = cpu_.tick();
    ppu_.tick(cycles);
    // apu->tick(cycles);
    // timer->tick(cycles);
}

} // namespace gameboy
