#include <memory/controller/mbc2.h>
#include <memory/address_range.h>
#include <util/mathutil.h>

gameboy::mbc2::mbc2(const std::vector<uint8_t>& rom, const gameboy::cartridge& rom_header)
    :mbc(rom, rom_header) { }

void gameboy::mbc2::select_rom_bank(const uint8_t data)
{
    rom_bank_ = data & 0x0Fu;
}

void gameboy::mbc2::control(const gameboy::address16& virtual_address, uint8_t data)
{
    constexpr address_range external_ram_enable_range(0x1FFFu);
    constexpr address_range rom_bank_select_range(0x2000u, 0x3FFFu);

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

void gameboy::mbc2::write(const gameboy::address16& virtual_address, uint8_t data)
{
    mbc::write(virtual_address, data & 0x0Fu);
}
