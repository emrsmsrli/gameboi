#include <cartridge.h>
#include <memory/address.h>

uint8_t gameboy::cartridge::read_rom(const address16& address) const
{
    // fixme mbc
    return rom_[address.value()];
}
void gameboy::cartridge::write_rom(const gameboy::address16& address, uint8_t data)
{
    // fixme mbc
    rom_[address.value()] = data;
}
uint8_t gameboy::cartridge::read_ram(const gameboy::address16& address) const
{
    // fixme mbc
    return ram_[address.value()];
}
void gameboy::cartridge::write_ram(const gameboy::address16& address, uint8_t data)
{
    // fixme mbc
    ram_[address.value()] = data;
}
