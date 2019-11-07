#include <chrono>
#include <thread>

#include <gameboy.h>

namespace gameboy {

gameboy::gameboy(const std::string_view rom_path)
    : cartridge_{rom_path},
      bus_{make_observer(cartridge_)},
      mmu_{make_observer(bus_)},
      cpu_{make_observer(bus_)},
      ppu_{make_observer(bus_)},
      apu_{make_observer(bus_)}
{
    mmu_.initialize();

    bus_.cartridge_ = make_observer(cartridge_);
    bus_.mmu_ = make_observer(mmu_);
    bus_.cpu_ = make_observer(cpu_);
    bus_.ppu_ = make_observer(ppu_);
    bus_.apu_ = make_observer(apu_);
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
