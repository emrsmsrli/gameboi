#include <cstdint>
#include <memory/AddressRange.h>
#include <memory/controller/MBC1.h>

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
        select_rom_bank(data);
    } else if(ram_bank_select_range.contains(virtual_address)) {
        select_ram_bank(data);
    } else if(memory_mode_select_range.contains(virtual_address)) {
        select_memory_mode(data);
    }
}

void gameboy::memory::controller::MBC1::select_memory_mode(uint8_t data)
{
    is_rom_banking_active = (data & 0x1u) == 0x0u;
}

void gameboy::memory::controller::MBC1::select_rom_bank(uint8_t data)
{
    rom_bank = data & 0x1Fu;
}

void gameboy::memory::controller::MBC1::select_ram_bank(uint8_t data)
{
    ram_bank = data & 0x03u;
}

uint32_t gameboy::memory::controller::MBC1::get_rom_bank() const
{
    const auto bank = [&]() {
        if(is_rom_banking_active) {
            return (ram_bank & 0x3u) << 0x5u | rom_bank;
        }

        return rom_bank;
    }();

    switch(bank) {
        case 0x00u:
        case 0x20u:
        case 0x40u:
        case 0x60u:
            // these banks are unusuable, must return the next one
            return bank;
        default:
            // we already have first bank between 0x0000-0x3FFF
            return bank - 1;
    }
}

uint32_t gameboy::memory::controller::MBC1::get_ram_bank() const
{
    if(is_rom_banking_active) {
        return 0u;
    }

    return ram_bank;
}
