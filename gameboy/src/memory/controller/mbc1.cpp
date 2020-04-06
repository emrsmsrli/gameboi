#include "gameboy/cartridge.h"
#include "gameboy/memory/controller/mbc1.h"
#include "gameboy/memory/address_range.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

void mbc1::control(const address16& address, const uint8_t data) noexcept
{
    constexpr address_range external_ram_enable_range{0x1FFFu};
    constexpr address_range rom_bank_select_range{0x2000u, 0x3FFFu};
    constexpr address_range ram_bank_select_range{0x4000u, 0x5FFFu};
    constexpr address_range memory_mode_select_range{0x6000u, 0x7FFFu};

    if(external_ram_enable_range.has(address)) {
        if(cartridge_->ram_bank_count() != 0u) {
            set_ram_enabled(data);
        }
    } else if(rom_bank_select_range.has(address)) {
        rom_bank_ = data & 0x1Fu;
        if(rom_bank_ == 0u) {
            rom_bank_ = 1u;
        }
    } else if(ram_bank_select_range.has(address)) {
        ram_bank_ = data & 0x03u;
    } else if(memory_mode_select_range.has(address)) {
        rom_banking_active_ = !bit::test(data, 0u);
    }
}

uint8_t mbc1::read_ram(const physical_address& address) const
{
    return cartridge_->ram()[address.value()];
}

void mbc1::write_ram(const physical_address& address, const uint8_t data)
{
    cartridge_->ram()[address.value()] = data;
}

} // namespace gameboy
