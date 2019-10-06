#include <memory/controller/MBC2.h>
#include <memory/AddressRange.h>
#include <util/Math.h>

gameboy::memory::controller::MBC2::MBC2(const std::vector<uint8_t>& rom, const gameboy::CartridgeInfo& rom_header)
        : MBC(rom, rom_header) { }

void gameboy::memory::controller::MBC2::select_rom_bank(const uint8_t data)
{
    rom_bank = data & 0x0Fu;
}

void gameboy::memory::controller::MBC2::control(const gameboy::memory::Address16& virtual_address, uint8_t data)
{
    constexpr AddressRange external_ram_enable_range(0x1FFFu);
    constexpr AddressRange rom_bank_select_range(0x2000u, 0x3FFFu);

    if(external_ram_enable_range.contains(virtual_address)) {
        if(!math::bit_test(virtual_address.get_value(), 0x0100u)) {
            set_external_ram_enabled(data);
        }
    } else if(rom_bank_select_range.contains(virtual_address)) {
        if(math::bit_test(virtual_address.get_value(), 0x0100u)) {
            select_rom_bank(data);
        }
    }
}

void gameboy::memory::controller::MBC2::write(const gameboy::memory::Address16& virtual_address, uint8_t data)
{
    MBC::write(virtual_address, data & 0x0Fu);
}
