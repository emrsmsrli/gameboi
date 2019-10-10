#include <gameboy.h>
#include <util/data_loader.h>

namespace {
    constexpr auto max_cycles = 70224;
    constexpr auto fps = 59.73f;
    constexpr auto delay = 1000.f / fps;
}

gameboy::gameboy::gameboy(const std::string_view rom_path)
        : memory_(std::make_shared<mmu>()),
          cpu_(std::make_unique<cpu>(memory_)),
          ppu_(std::make_unique<ppu>(memory_))
{
    const auto rom_data = util::data_loader::load(rom_path);
    memory_->load_rom(rom_data);
}

void gameboy::gameboy::start()
{
    memory_->initialize();
    cpu_->initialize();

    while(true) {
        const auto cycles = cpu_->tick();
        ppu_->tick(cycles);

        // checkPowerMode();
        // checkInterrupts();

        // ppu->update(cycles, interrupt_master_enable);
        // apu->update(cycles);
        // timer.update(cycles);
    }
}
