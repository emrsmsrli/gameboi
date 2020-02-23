#include <cstdint>

#include "gameboy/memory/controller/mbc5.h"
#include "gameboy/memory/address_range.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

void mbc5::control(const address16& address, const uint8_t data) noexcept
{
    constexpr address_range external_ram_enable_range{0x1FFFu};
    constexpr address_range rom_bank_select_low_range{0x2000u, 0x2FFFu};
    constexpr address_range rom_bank_select_high_range{0x3000u, 0x3FFFu};
    constexpr address_range ram_bank_select_range{0x4000u, 0x5FFFu};
    constexpr address_range memory_mode_select_range{0x6000u, 0x7FFFu};

    if(external_ram_enable_range.has(address)) {
        set_xram_enabled(data);
    } else if(rom_bank_select_low_range.has(address)) {
        rom_bank = (rom_bank & ~0x000Fu) | data;
    } else if(rom_bank_select_high_range.has(address)) {
        rom_bank = (rom_bank & ~0x0010u) | ((data & 0x1u) << 8u);
    } else if(ram_bank_select_range.has(address)) {
        ram_bank = data & 0x0Fu;
    } else if(memory_mode_select_range.has(address)) {
        rom_banking_active = !bit_test(data, 0x1u);
    }
}

uint8_t mbc5::read_ram(const std::vector<uint8_t>& ram, const physical_address& address) const
{
    return ram[address.value()];
}

void mbc5::write_ram(std::vector<uint8_t>& ram, const physical_address& address, const uint8_t data) const
{
    ram[address.value()] = data;
}

} // namespace gameboy
