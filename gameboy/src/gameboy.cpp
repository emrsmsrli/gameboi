#include <spdlog/spdlog.h>

#include "gameboy/gameboy.h"
#include "gameboy/version.h"

namespace gameboy {

gameboy::gameboy(const filesystem::path& rom_path)
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

void gameboy::tick()
{
    const auto cycles = cpu_.tick();

    if(!cpu_.is_stopped()) {
        timer_.tick(cycles);
        apu_.tick(cycles);
        ppu_.tick(cycles);
        link_.tick(cycles);
    }

    cpu_.process_interrupts();
}

void gameboy::tick_one_frame()
{
    while(mmu_.read(ppu::ly_addr) != 0x00u) {
#if WITH_DEBUGGER
        if(!tick_enabled) {
            return;
        }
#endif //WITH_DEBUGGER

        tick();
    }

    while(mmu_.read(ppu::ly_addr) < 144) {
#if WITH_DEBUGGER
        if(!tick_enabled) {
            return;
        }
#endif //WITH_DEBUGGER

        tick();
    }
}

} // namespace gameboy
