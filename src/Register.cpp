
#include "Register.h"

void gameboy::HalfRegister::set(uint8_t index)
{
    reg.bits |= 0x1u << (index + offset);
}

void gameboy::HalfRegister::reset(uint8_t index)
{
    reg.bits &= ~(0x1u << (index + offset));
}

void gameboy::HalfRegister::flip(uint8_t index)
{
    reg.bits ^= 0x1u << (index + offset);
}

bool gameboy::HalfRegister::test(uint8_t index) const
{
    return reg.bits & (0x1u << (index + offset));
}

bool gameboy::HalfRegister::all() const
{
    return get() == 0xFF;
}

bool gameboy::HalfRegister::any() const
{
    return !none();
}

bool gameboy::HalfRegister::none() const
{
    return get() == 0x00;
}

uint8_t gameboy::HalfRegister::get() const
{
    return ((reg.bits >> offset) & 0xFF);
}

gameboy::HalfRegister& gameboy::HalfRegister::operator=(uint8_t value)
{
    const auto prev = reg.bits & ~(0xFF << offset);
    const auto newo = (value << offset);
    reg.bits = prev | newo;
    return *this;
}

void gameboy::Register::set(uint8_t index)
{
    bits |= 0x1u << index;
}

void gameboy::Register::reset(uint8_t index)
{
    bits &= ~(0x1u << index);
}

void gameboy::Register::flip(uint8_t index)
{
    bits ^= 0x1u << index;
}

bool gameboy::Register::test(uint8_t index) const
{
    return bits & (0x1u << index);
}

bool gameboy::Register::all() const
{
    return bits == 0xFFFF;
}

bool gameboy::Register::any() const
{
    return !none();
}

bool gameboy::Register::none() const
{
    return bits == 0x00;
}

gameboy::Register& gameboy::Register::operator=(uint16_t value)
{
    bits = value;
    return *this;
}

gameboy::Register& gameboy::Register::operator++()
{
    ++bits;
    return *this;
}

gameboy::Register& gameboy::Register::operator--()
{
    --bits;
    return *this;
}

gameboy::Register& gameboy::Register::operator+=(int16_t value)
{
    bits += value;
    return *this;
}

gameboy::Register& gameboy::Register::operator-=(int16_t value)
{
    bits -= value;
    return *this;
}
