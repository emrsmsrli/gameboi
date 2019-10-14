#include <memory/controller/mbc2.h>
#include <memory/address_range.h>
#include <util/mathutil.h>

void gameboy::mbc2::control(const gameboy::address16& address, uint8_t data) noexcept
{
    constexpr address_range external_ram_enable_range(0x1FFFu);
    constexpr address_range rom_bank_select_range(0x2000u, 0x3FFFu);

    if(external_ram_enable_range.contains(address)) {
        if(!math::bit_test(address.value(), 0x0100u)) {
            set_xram_enabled(data);
        }
    } else if(rom_bank_select_range.contains(address)) {
        if(math::bit_test(address.value(), 0x0100u)) {
            rom_bank = data & 0x0Fu;
        }
    }
}

uint8_t gameboy::mbc2::read_ram(const std::vector<uint8_t>& ram, const size_t address) const noexcept
{
    return ram[address];
}

void gameboy::mbc2::write_ram(std::vector<uint8_t>& ram, const size_t address, uint8_t data) const noexcept
{
    ram[address] = data & 0x0Fu;
}
