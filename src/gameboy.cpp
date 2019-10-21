#include <chrono>
#include <thread>

#include <gameboy.h>

namespace gameboy {

constexpr auto max_cycles = 70224;
constexpr auto fps = 59.73f;
constexpr auto delay = 1000.f / fps;

gameboy::gameboy(const std::string_view rom_path)
    : cartridge_(rom_path),
      bus_(bus{make_observer(cartridge_)}),
      mmu_(mmu{make_observer(bus_)}),
      cpu_(cpu{make_observer(bus_)}),
      ppu_(ppu{make_observer(bus_)}),
      apu_(apu{make_observer(bus_)})
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
    while(true) {
        tick();

        using namespace std::chrono;
        // const auto ns = duration_cast<nanoseconds>(milliseconds{16.7f});
        std::this_thread::sleep_for(nanoseconds{16700});
        break;
    }
}
void gameboy::tick()
{
    const auto cycles = cpu_.tick();
    ppu_.tick(cycles);

    // checkPowerMode();
    // checkInterrupts();

    // ppu->update(cycles, interrupt_master_enable);
    // apu->update(cycles);
    // timer.update(cycles);
}

} // namespace gameboy
