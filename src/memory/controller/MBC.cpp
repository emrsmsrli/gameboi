//
// Created by Emre Şimşirli on 28.08.2019.
//

#include <array>
#include "memory/controller/MBC.h"
#include "memory/Address.h"
#include "CartridgeInfo.h"

gameboy::memory::controller::MBC::MBC(const std::vector<uint8_t>& rom, const CartridgeInfo& rom_header)
{
    constexpr std::array<uint32_t, 11> rom_size_to_banks{2, 4, 8, 16, 32, 64, 128, 256, 72, 80, 96};

    is_cgb = rom_header.cgb_support != CartridgeInfo::GameBoyColorSupport::no_support;

    n_rom_banks = rom_size_to_banks[static_cast<std::size_t>(rom_header.rom_size)];
    n_video_ram_banks = is_cgb ? 2 : 1;
    n_external_ram_banks = rom_header.ram_size == CartridgeInfo::RamSize::kb_32 ? 4 : 1;
    n_working_ram_banks = is_cgb ? 8 : 2;

    const auto total_memory_size =
            16_kb * n_rom_banks +           // 0000-7FFF - rom bank 0-n
            8_kb * n_video_ram_banks +      // 8000-9FFF - vram bank 0-1
            8_kb * n_external_ram_banks +   // A000-BFFF - xram bank 0-n
            4_kb * n_working_ram_banks +    // C000-DFFF - wram bank 0-7
            8_kb;                           // E000-FFFF - high ram

    memory.reserve(total_memory_size);

    std::copy(begin(rom), end(rom), std::back_inserter(memory));
}

uint8_t gameboy::memory::controller::MBC::read(const Address16& virtual_address) const
{
    // todo consider banks
    const auto physical_address = to_physical_address(virtual_address);
    return memory[physical_address.get_value()];
}

void gameboy::memory::controller::MBC::write(const Address16& virtual_address, uint8_t data)
{
    // todo consider banks
    const auto physical_address = to_physical_address(virtual_address);
    memory[physical_address.get_value()] = data;
}

gameboy::memory::Address16 gameboy::memory::controller::MBC::to_physical_address(
        const gameboy::memory::Address16& virtual_address) const
{
    // todo implement
    return virtual_address;
}
