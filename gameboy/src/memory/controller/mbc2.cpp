#include "gameboy/memory/controller/mbc2.h"
#include "gameboy/memory/address_range.h"
#include "gameboy/cartridge.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

[[nodiscard]] bool is_invalid_ram_address(const physical_address& address) noexcept
{
    constexpr address_range invalid_ram_range{0xA200u, 0xBFFFu};
    return invalid_ram_range.has(address16(address.value()));
}

void mbc2::control(const address16& address, const uint8_t data) noexcept
{
    constexpr address_range external_ram_enable_range{0x1FFFu};
    constexpr address_range rom_bank_select_range{0x2000u, 0x3FFFu};

    if(external_ram_enable_range.has(address)) {
        if(!mask_test(address.value(), 0x0100u)) {
            set_ram_enabled(data);
        }
    } else if(rom_bank_select_range.has(address)) {
        if(mask_test(address.value(), 0x0100u)) {
            rom_bank_ = data & 0x0Fu;
            if(rom_bank_ == 0u) {
                rom_bank_ = 1u;
            }

            rom_bank_ &= cartridge_->rom_bank_count() - 1u;
        }
    }
}

uint8_t mbc2::read_ram(const physical_address& address) const
{
    if(is_invalid_ram_address(address)) {
        return 0x00u;
    }

    return cartridge_->ram()[address.value()];
}

void mbc2::write_ram(const physical_address& address, const uint8_t data)
{
    if(is_invalid_ram_address(address)) {
        return;
    }

    cartridge_->ram()[address.value()] = data & 0x0Fu;
}

} // namespace gameboy
