
#include "cpu/Register8.h"

gameboy::cpu::Register8& gameboy::cpu::Register8::operator=(uint8_t value)
{
    bits = value;
    return *this;
}

gameboy::cpu::Register8& gameboy::cpu::Register8::operator++()
{
    ++bits;
    return *this;
}

gameboy::cpu::Register8& gameboy::cpu::Register8::operator--()
{
    --bits;
    return *this;
}

gameboy::cpu::Register8& gameboy::cpu::Register8::operator+=(uint8_t value)
{
    bits += value;
    return *this;
}

gameboy::cpu::Register8& gameboy::cpu::Register8::operator+=(const gameboy::cpu::Register8& reg)
{
    bits += reg.bits;
    return *this;
}

gameboy::cpu::Register8& gameboy::cpu::Register8::operator-=(uint8_t value)
{
    bits -= value;
    return *this;
}

gameboy::cpu::Register8& gameboy::cpu::Register8::operator-=(const gameboy::cpu::Register8& reg)
{
    bits -= reg.bits;
    return *this;
}

uint16_t gameboy::cpu::Register8::operator+(uint16_t value) const
{
    return bits + value;
}

uint16_t gameboy::cpu::Register8::operator+(const gameboy::cpu::Register8& reg) const
{
    return static_cast<uint16_t>(bits) + reg.bits;
}

uint16_t gameboy::cpu::Register8::operator-(uint16_t value) const
{
    return bits - value;
}

uint16_t gameboy::cpu::Register8::operator-(const gameboy::cpu::Register8& reg) const
{
    return static_cast<uint16_t>(bits) - reg.bits;
}

gameboy::cpu::Register8& gameboy::cpu::Register8::operator&=(uint8_t value)
{
    bits &= value;
    return *this;
}

gameboy::cpu::Register8& gameboy::cpu::Register8::operator&=(const gameboy::cpu::Register8& reg)
{
    bits &= reg.bits;
    return *this;
}

gameboy::cpu::Register8& gameboy::cpu::Register8::operator|=(uint8_t value)
{
    bits |= value;
    return *this;
}

gameboy::cpu::Register8& gameboy::cpu::Register8::operator|=(const gameboy::cpu::Register8& reg)
{
    bits |= reg.bits;
    return *this;
}

gameboy::cpu::Register8& gameboy::cpu::Register8::operator^=(uint8_t value)
{
    bits ^= value;
    return *this;
}

gameboy::cpu::Register8& gameboy::cpu::Register8::operator^=(const gameboy::cpu::Register8& reg)
{
    bits ^= reg.bits;
    return *this;
}

gameboy::cpu::Register8 gameboy::cpu::Register8::operator&(uint8_t value) const
{
    return Register8(bits & value);
}

gameboy::cpu::Register8 gameboy::cpu::Register8::operator&(const gameboy::cpu::Register8& reg) const
{
    return Register8(bits & reg.bits);
}

gameboy::cpu::Register8 gameboy::cpu::Register8::operator|(uint8_t value) const
{
    return Register8(bits | value);
}

gameboy::cpu::Register8 gameboy::cpu::Register8::operator|(const gameboy::cpu::Register8& reg) const
{
    return Register8(bits | reg.bits);
}

gameboy::cpu::Register8 gameboy::cpu::Register8::operator^(uint8_t value) const
{
    return Register8(bits ^ value);
}

gameboy::cpu::Register8 gameboy::cpu::Register8::operator^(const gameboy::cpu::Register8& reg) const
{
    return Register8(bits ^ reg.bits);
}

gameboy::cpu::Register8 gameboy::cpu::Register8::operator~() const
{
    return Register8(~bits);
}

bool gameboy::cpu::Register8::operator==(uint8_t value) const
{
    return bits == value;
}

bool gameboy::cpu::Register8::operator==(const gameboy::cpu::Register8& reg) const
{
    return bits == reg.bits;
}

bool gameboy::cpu::Register8::operator!=(uint8_t value) const
{
    return bits != value;
}

bool gameboy::cpu::Register8::operator!=(const gameboy::cpu::Register8& reg) const
{
    return bits != reg.bits;
}

bool gameboy::cpu::Register8::operator>(uint8_t value) const
{
    return bits > value;
}

bool gameboy::cpu::Register8::operator>(const gameboy::cpu::Register8& reg) const
{
    return bits > reg.bits;
}

bool gameboy::cpu::Register8::operator<(uint8_t value) const
{
    return bits < value;
}

bool gameboy::cpu::Register8::operator<(const gameboy::cpu::Register8& reg) const
{
    return bits < reg.bits;
}

bool gameboy::cpu::Register8::operator>=(uint8_t value) const
{
    return bits >= value;
}

bool gameboy::cpu::Register8::operator>=(const gameboy::cpu::Register8& reg) const
{
    return bits >= reg.bits;
}

bool gameboy::cpu::Register8::operator<=(uint8_t value) const
{
    return bits <= value;
}

bool gameboy::cpu::Register8::operator<=(const gameboy::cpu::Register8& reg) const
{
    return bits <= reg.bits;
}
