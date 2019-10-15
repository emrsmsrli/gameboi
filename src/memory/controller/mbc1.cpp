#include <cstdint>

#include <memory/controller/mbc1.h>
#include <memory/address_range.h>
#include <util/mathutil.h>

void gameboy::mbc1::control(const gameboy::address16& address, const uint8_t data) noexcept
{
    constexpr address_range external_ram_enable_range(0x1FFFu);
    constexpr address_range rom_bank_select_range(0x2000u, 0x3FFFu);
    constexpr address_range ram_bank_select_range(0x4000u, 0x5FFFu);
    constexpr address_range memory_mode_select_range(0x6000u, 0x7FFFu);

    if(external_ram_enable_range.contains(address)) {
        set_xram_enabled(data);
    } else if(rom_bank_select_range.contains(address)) {
        rom_bank = data & 0x1Fu;
    } else if(ram_bank_select_range.contains(address)) {
        ram_bank = data & 0x03u;
    } else if(memory_mode_select_range.contains(address)) {
        rom_banking_active = !math::bit_test(data, 0x1u);
    }
}

uint8_t gameboy::mbc1::read_ram(const std::vector<uint8_t>& ram, const physical_address& address) const noexcept
{
    return ram[address.value()];
}

void gameboy::mbc1::write_ram(
    std::vector<uint8_t>& ram,
    const physical_address& address,
    const uint8_t data) const noexcept
{
    ram[address.value()] = data;
}
