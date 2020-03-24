#include "gameboy/memory/controller/mbc5.h"
#include "gameboy/memory/address_range.h"
#include "gameboy/cartridge.h"

namespace gameboy {

void mbc5::control(const address16& address, const uint8_t data) noexcept
{
    constexpr address_range external_ram_enable_range{0x1FFFu};
    constexpr address_range rom_bank_select_low_range{0x2000u, 0x2FFFu};
    constexpr address_range rom_bank_select_high_range{0x3000u, 0x3FFFu};
    constexpr address_range ram_bank_select_range{0x4000u, 0x5FFFu};

    if(external_ram_enable_range.has(address)) {
        if(cartridge_->ram_bank_count() != 0u) {
            set_ram_enabled(data);
        }
    } else if(rom_bank_select_low_range.has(address)) {
        rom_bank_ = (rom_bank_ & 0x0100u) | data;
        rom_bank_ &= cartridge_->rom_bank_count() - 1u;
    } else if(rom_bank_select_high_range.has(address)) {
        rom_bank_ = (rom_bank_ & 0x00FFu) | ((data & 0x01u) << 8u);
        rom_bank_ &= cartridge_->rom_bank_count() - 1u;
    } else if(ram_bank_select_range.has(address)) {
        ram_bank_ = data & 0x0Fu;
        ram_bank_ &= cartridge_->ram_bank_count() - 1u;
    }
}

uint8_t mbc5::read_ram(const physical_address& address) const
{
    return cartridge_->ram()[address.value()];
}

void mbc5::write_ram(const physical_address& address, const uint8_t data)
{
    cartridge_->ram()[address.value()] = data;
}

} // namespace gameboy
