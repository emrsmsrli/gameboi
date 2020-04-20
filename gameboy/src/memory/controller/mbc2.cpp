#include "gameboy/memory/controller/mbc2.h"

#include "gameboy/cartridge.h"
#include "gameboy/memory/address_range.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

[[nodiscard]] size_t wrap_address(const physical_address& address) noexcept
{
    return address.value() & 0x1FFu;
}

void mbc2::control(const address16& address, const uint8_t data) noexcept
{
    constexpr address_range control_range{0x3FFFu};

    if(control_range.has(address)) {
        if(!bit::test(address.value(), 8u)) {
            set_ram_enabled(data);
        } else {
            rom_bank_ = data & 0x0Fu;
            if(rom_bank_ == 0u) {
                rom_bank_ = 1u;
            }
        }
    }
}

uint8_t mbc2::read_ram(const physical_address& address) const
{
    return cartridge_->ram()[wrap_address(address)] | 0xF0u;
}

void mbc2::write_ram(const physical_address& address, const uint8_t data)
{
    cartridge_->ram()[wrap_address(address)] = data & 0x0Fu;
}

} // namespace gameboy
