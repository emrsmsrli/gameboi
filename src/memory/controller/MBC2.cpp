#include <memory/controller/MBC2.h>

gameboy::memory::controller::MBC2::MBC2(const std::vector<uint8_t>& rom, const gameboy::CartridgeInfo& rom_header)
        : MBC(rom, rom_header) { }

void gameboy::memory::controller::MBC2::control(const gameboy::memory::Address16& virtual_address, uint8_t data)
{

}
