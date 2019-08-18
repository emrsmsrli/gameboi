
#include "memory/Address.h"
#include "cpu/Register16.h"

gameboy::memory::Address8 gameboy::memory::make_address(uint8_t address)
{
    return Address8(address);
}

gameboy::memory::Address16 gameboy::memory::make_address(uint16_t address)
{
    return Address16(address);
}

gameboy::memory::Address16 gameboy::memory::make_address(const gameboy::cpu::Register16& reg)
{
    return Address16(reg.get_value());
}
