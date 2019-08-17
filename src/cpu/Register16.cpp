
#include "cpu/Register16.h"

uint16_t gameboy::cpu::Register16::get_value() const
{
    const uint16_t value = high.get_value();
    return (value << 8) | low.get_value();
}

gameboy::cpu::Register16& gameboy::cpu::Register16::operator=(uint16_t value)
{
    low = value & 0xFF;
    high = (value >> 8) & 0xFF;
    return *this;
}

gameboy::cpu::Register16& gameboy::cpu::Register16::operator++()
{
    *this += 1;
    return *this;
}

gameboy::cpu::Register16& gameboy::cpu::Register16::operator--()
{
    *this -= 1;
    return *this;
}

gameboy::cpu::Register16& gameboy::cpu::Register16::operator+=(uint16_t value)
{
    *this = *this + value;
    return *this;
}

gameboy::cpu::Register16& gameboy::cpu::Register16::operator+=(const gameboy::cpu::Register16& r)
{
    *this += r.get_value();
    return *this;
}

gameboy::cpu::Register16& gameboy::cpu::Register16::operator-=(uint16_t value)
{
    *this = *this - value;
    return *this;
}

gameboy::cpu::Register16& gameboy::cpu::Register16::operator-=(const gameboy::cpu::Register16& r)
{
    *this -= r.get_value();
    return *this;
}

uint32_t gameboy::cpu::Register16::operator+(uint32_t value) const
{
    uint32_t this_value = get_value();
    return this_value + value;
}

uint32_t gameboy::cpu::Register16::operator+(const gameboy::cpu::Register16& reg) const
{
    uint32_t this_value = get_value();
    return this_value + reg.get_value();
}

uint32_t gameboy::cpu::Register16::operator-(uint32_t value) const
{
    uint32_t this_value = get_value();
    return this_value - value;
}

uint32_t gameboy::cpu::Register16::operator-(const gameboy::cpu::Register16& reg) const
{
    uint32_t this_value = get_value();
    return this_value - reg.get_value();
}

gameboy::cpu::Register16& gameboy::cpu::Register16::operator&=(uint16_t value)
{
    *this = get_value() & value;
    return *this;
}

gameboy::cpu::Register16& gameboy::cpu::Register16::operator&=(const gameboy::cpu::Register16& reg)
{
    *this &= reg.get_value();
    return *this;
}

gameboy::cpu::Register16& gameboy::cpu::Register16::operator|=(uint16_t value)
{
    *this = get_value() | value;
    return *this;
}

gameboy::cpu::Register16& gameboy::cpu::Register16::operator|=(const gameboy::cpu::Register16& reg)
{
    *this |= reg.get_value();
    return *this;
}

gameboy::cpu::Register16& gameboy::cpu::Register16::operator^=(uint16_t value)
{
    *this = get_value() ^ value;
    return *this;
}

gameboy::cpu::Register16& gameboy::cpu::Register16::operator^=(const gameboy::cpu::Register16& reg)
{
    *this ^= reg.get_value();
    return *this;
}

gameboy::cpu::Register16 gameboy::cpu::Register16::operator&(uint16_t value) const
{
    return Register16(get_value() & value);
}

gameboy::cpu::Register16 gameboy::cpu::Register16::operator&(const gameboy::cpu::Register16& reg) const
{
    return Register16(get_value() & reg.get_value());
}

gameboy::cpu::Register16 gameboy::cpu::Register16::operator|(uint16_t value) const
{
    return Register16(get_value() | value);
}

gameboy::cpu::Register16 gameboy::cpu::Register16::operator|(const gameboy::cpu::Register16& reg) const
{
    return Register16(get_value() | reg.get_value());
}

gameboy::cpu::Register16 gameboy::cpu::Register16::operator^(uint16_t value) const
{
    return Register16(get_value() ^ value);
}

gameboy::cpu::Register16 gameboy::cpu::Register16::operator^(const gameboy::cpu::Register16& reg) const
{
    return Register16(get_value() ^ reg.get_value());
}

gameboy::cpu::Register16 gameboy::cpu::Register16::operator~() const
{
    return Register16(~get_value());
}

bool gameboy::cpu::Register16::operator==(uint16_t value) const
{
    return get_value() == value;
}

bool gameboy::cpu::Register16::operator==(const gameboy::cpu::Register16& reg) const
{
    return get_value() == reg.get_value();
}

bool gameboy::cpu::Register16::operator!=(uint16_t value) const
{
    return get_value() != value;
}

bool gameboy::cpu::Register16::operator!=(const gameboy::cpu::Register16& reg) const
{
    return get_value() != reg.get_value();
}

bool gameboy::cpu::Register16::operator>(uint16_t value) const
{
    return get_value() > value;
}

bool gameboy::cpu::Register16::operator>(const gameboy::cpu::Register16& reg) const
{
    return get_value() > reg.get_value();
}

bool gameboy::cpu::Register16::operator<(uint16_t value) const
{
    return get_value() < value;
}

bool gameboy::cpu::Register16::operator<(const gameboy::cpu::Register16& reg) const
{
    return get_value() < reg.get_value();
}

bool gameboy::cpu::Register16::operator>=(uint16_t value) const
{
    return get_value() >= value;
}

bool gameboy::cpu::Register16::operator>=(const gameboy::cpu::Register16& reg) const
{
    return get_value() >= reg.get_value();
}

bool gameboy::cpu::Register16::operator<=(uint16_t value) const
{
    return get_value() <= value;
}

bool gameboy::cpu::Register16::operator<=(const gameboy::cpu::Register16& reg) const
{
    return get_value() <= reg.get_value();
}
