#include <cartridge.h>
#include <memory/address.h>

uint8_t gameboy::cartridge::read_rom(const address16& address) const
{
    // fixme mbc
    return rom_[address.get_value()];
}
void gameboy::cartridge::write_rom(const gameboy::address16& address, uint8_t data)
{
    // fixme mbc
    rom_[address.get_value()] = data;
}
uint8_t gameboy::cartridge::read_ram(const gameboy::address16& address) const
{
    // fixme mbc
    return ram_[address.get_value()];
}
void gameboy::cartridge::write_ram(const gameboy::address16& address, uint8_t data)
{
    // fixme mbc
    ram_[address.get_value()] = data;
}
