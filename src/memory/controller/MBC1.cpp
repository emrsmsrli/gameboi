//
// Created by Emre Şimşirli on 28.08.2019.
//

#include <cstdint>
#include "memory/AddressRange.h"
#include "memory/controller/MBC1.h"

gameboy::memory::controller::MBC1::MBC1(const std::vector<uint8_t>& rom, const gameboy::CartridgeInfo& rom_header)
        : MBC(rom, rom_header) { }

uint8_t gameboy::memory::controller::MBC1::read(const gameboy::memory::Address16& address) const
{
    constexpr AddressRange external_ram_range(Map::ram_external_start, Map::ram_external_end);
    if(external_ram_range.contains(address) && !is_external_ram_enabled) {
        return 0xFFu;
    }

    return MBC::read(address);
}

void gameboy::memory::controller::MBC1::write(const gameboy::memory::Address16& address, uint8_t data)
{
    if(AddressRange(0x0000, 0x1FFF).contains(address)) {
        is_external_ram_enabled = (data & 0x0Fu) == 0x0A;
    } else if(AddressRange(0x2000, 0x3FFF).contains(address)) {
        // todo set selected_rom_bank
    }

    MBC::write(address, data);
}
