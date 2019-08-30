
#include "memory/Address.h"
#include "cpu/Register16.h"

gameboy::memory::Address16 gameboy::memory::make_address(Map location)
{
    return make_address(static_cast<uint16_t>(location));
}

gameboy::memory::Address16 gameboy::memory::make_address(const gameboy::cpu::Register16& reg)
{
    return Address16(reg.get_value());
}
