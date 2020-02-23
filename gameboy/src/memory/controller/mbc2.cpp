#include "gameboy/memory/controller/mbc2.h"
#include "gameboy/memory/address_range.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

void mbc2::control(const address16& address, const uint8_t data) noexcept
{
    constexpr address_range external_ram_enable_range{0x1FFFu};
    constexpr address_range rom_bank_select_range{0x2000u, 0x3FFFu};

    if(external_ram_enable_range.has(address)) {
        if(!mask_test(address.value(), 0x0100u)) {
            set_xram_enabled(data);
        }
    } else if(rom_bank_select_range.has(address)) {
        if(mask_test(address.value(), 0x0100u)) {
            rom_bank = data & 0x0Fu;
            if(rom_bank == 0u) {
                rom_bank = 1u;
            }
        }
    }
}

uint8_t mbc2::read_ram(const std::vector<uint8_t>& ram, const physical_address& address) const
{
    return ram[address.value()];
}

void mbc2::write_ram(std::vector<uint8_t>& ram, const physical_address& address, const uint8_t data) const
{
    ram[address.value()] = data & 0x0Fu;
}

} // namespace gameboy
