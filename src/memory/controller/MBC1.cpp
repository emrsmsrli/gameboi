//
// Created by Emre Şimşirli on 28.08.2019.
//

#include <cstdint>
#include "memory/AddressRange.h"
#include "memory/controller/MBC1.h"

gameboy::memory::controller::MBC1::MBC1(const std::vector<uint8_t>& rom, const gameboy::CartridgeInfo& rom_header)
        : MBC(rom, rom_header) { }

void gameboy::memory::controller::MBC1::control(const gameboy::memory::Address16& virtual_address, uint8_t data)
{
    constexpr AddressRange external_ram_enable_range(0x0000u, 0x1FFFu);
    constexpr AddressRange rom_bank_select_range(0x2000u, 0x3FFFu);
    constexpr AddressRange ram_bank_select_range(0x4000u, 0x5FFFu);
    constexpr AddressRange memory_mode_select_range(0x6000u, 0x7FFFu);

    if(external_ram_enable_range.contains(virtual_address)) {
        is_external_ram_enabled = (data & 0x0Fu) == 0x0Au;
    } else if(rom_bank_select_range.contains(virtual_address)) {
        if(mode == Mode::rom_banking) {
            selected_rom_bank = (selected_rom_bank & 0xE0u) | (data & 0x1Fu);
        } else {
            selected_rom_bank = data & 0x1Fu;
        }

        correct_rom_bank();
    } else if(ram_bank_select_range.contains(virtual_address)) {
        if(mode == Mode::rom_banking) {
            selected_rom_bank = ((data & 0x03u) << 0x5u) | (selected_rom_bank & 0x1Fu);
            correct_rom_bank();
        } else {
            selected_external_ram_bank = data & 0x03u;
        }
    } else if(memory_mode_select_range.contains(virtual_address)) {
        select_memory_mode(data);
    }
}

void gameboy::memory::controller::MBC1::correct_rom_bank()
{
    switch(selected_rom_bank) {
        case 0x00:
        case 0x20:
        case 0x40:
        case 0x60:
            ++selected_rom_bank;
        default:
            break;
    }

    --selected_rom_bank;
}

void gameboy::memory::controller::MBC1::select_memory_mode(uint8_t data)
{
    mode = static_cast<Mode>(data & 0x1u);
}
