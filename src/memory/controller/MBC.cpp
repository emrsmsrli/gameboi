//
// Created by Emre Şimşirli on 28.08.2019.
//

#include <array>
#include "memory/controller/MBC.h"
#include "memory/Address.h"
#include "CartridgeInfo.h"

gameboy::memory::controller::MBC::MBC(const std::vector<uint8_t>& rom, const CartridgeInfo& rom_header)
{
    constexpr std::array<int32_t, 11> rom_size_to_banks{2, 4, 8, 16, 32, 64, 128, 256, 72, 80, 96};
    n_rom_banks = rom_size_to_banks[static_cast<std::size_t>(rom_header.rom_size)] - 1;
    n_ram_banks = rom_header.ram_size == CartridgeInfo::RamSize::kb_32 ? 4 : 1;

    // todo also reserve for rom and ram banks
    memory.reserve(rom.size());

    std::copy(begin(rom), end(rom), std::back_inserter(memory));
}

uint8_t gameboy::memory::controller::MBC::read(const Address16& virtual_address) const
{
    // todo consider banks
    return memory[virtual_address.get_value()];
}

void gameboy::memory::controller::MBC::write(const Address16& virtual_address, uint8_t data)
{
    // todo consider banks
    memory[virtual_address.get_value()] = data;
}

gameboy::memory::Address16
gameboy::memory::controller::MBC::to_physical_address(const gameboy::memory::Address16& virtual_address) const
{
    // todo implement
    return gameboy::memory::Address16();
}
