#include "gameboy/memory/controller/mbc_regular.h"

#include "gameboy/cartridge.h"

uint8_t gameboy::mbc_regular::read_ram(const gameboy::physical_address& address) const
{
    return cartridge_->ram()[address.value()];
}

void gameboy::mbc_regular::write_ram(const gameboy::physical_address& address, const uint8_t data) const
{
    cartridge_->ram()[address.value()] = data;
}
