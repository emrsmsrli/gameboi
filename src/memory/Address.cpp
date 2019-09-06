#include <memory/Address.h>
#include <cpu/Register16.h>

constexpr gameboy::memory::Address16 gameboy::memory::make_address(const gameboy::cpu::Register16& reg)
{
    return Address16(reg.get_value());
}
