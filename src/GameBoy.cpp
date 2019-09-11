#include <GameBoy.h>
#include <util/DataLoader.h>

namespace {
    constexpr auto max_cycles = 70224;
    constexpr auto fps = 59.73f;
    constexpr auto delay = 1000.f / fps;
}

gameboy::GameBoy::GameBoy(std::string_view rom_path)
        : memory(std::make_shared<memory::MMU>()),
          cpu(std::make_unique<cpu::CPU>(memory)),
          ppu(std::make_unique<ppu::PPU>(memory))
{
    const auto rom_data = util::data_loader::load(rom_path);
    memory->load_rom(rom_data);
}

void gameboy::GameBoy::start()
{
    memory->initialize();
    cpu->initialize();

    while(true) {
        cpu->tick();
        ppu->tick();

}
