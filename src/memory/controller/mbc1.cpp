#include <cstdint>
#include <memory/address_range.h>
#include <memory/controller/mbc1.h>
#include <util/math.h>

gameboy::mbc1::mbc1(const std::vector<uint8_t>& rom, const gameboy::cartridge& rom_header)
    :mbc(rom, rom_header) { }

void gameboy::mbc1::control(const gameboy::address16& virtual_address, uint8_t data)
{
    constexpr address_range external_ram_enable_range(0x1FFFu);
    constexpr address_range rom_bank_select_range(0x2000u, 0x3FFFu);
    constexpr address_range ram_bank_select_range(0x4000u, 0x5FFFu);
    constexpr address_range memory_mode_select_range(0x6000u, 0x7FFFu);

    if(external_ram_enable_range.contains(virtual_address)) {
        set_external_ram_enabled(data);
    } else if(rom_bank_select_range.contains(virtual_address)) {
        select_rom_bank(data);
    } else if(ram_bank_select_range.contains(virtual_address)) {
        select_ram_bank(data);
    } else if(memory_mode_select_range.contains(virtual_address)) {
        select_memory_mode(data);
    }
}

void gameboy::mbc1::select_memory_mode(uint8_t data)
{
    is_rom_banking_active_ = !math::bit_test(data, 0x1u);
}

void gameboy::mbc1::select_rom_bank(uint8_t data)
{
    rom_bank_ = data & 0x1Fu;
}

void gameboy::mbc1::select_ram_bank(uint8_t data)
{
    ram_bank_ = data & 0x03u;
}

uint32_t gameboy::mbc1::get_rom_bank() const
{
    const auto bank = [&]() {
        if(is_rom_banking_active_) {
            return (ram_bank_ & 0x3u) << 0x5u | rom_bank_;
        }

        return rom_bank_;
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

uint32_t gameboy::mbc1::get_ram_bank() const
{
    if(is_rom_banking_active_) {
        return 0u;
    }

    return ram_bank_;
}
