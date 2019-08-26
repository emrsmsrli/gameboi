
#include "GameBoy.h"
#include "util/DataLoader.h"

namespace {
    constexpr auto max_cycles = 70224;
    constexpr auto fps = 59.73f;
    constexpr auto delay = 1000.f / fps;
}

gameboy::GameBoy::GameBoy(std::string_view rom_path) :
    cpu(std::make_unique<cpu::CPU>()),
    memory(std::make_unique<memory::MMU>(rom_path))
{
    const auto rom_data = util::data_loader::load(rom_path);
}

void gameboy::GameBoy::start()
{

}
