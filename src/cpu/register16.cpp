#include <cpu/register16.h>
#include <memory/address.h>

uint16_t gameboy::register16::value() const
{
    const uint16_t h = high_.value();
    return (h << 8u) | low_.value();
}

gameboy::register16& gameboy::register16::operator=(uint16_t val)
{
    low_ = val & 0xFFu;
    high_ = (val >> 0x8u) & 0xFFu;
    return *this;
}

gameboy::register16& gameboy::register16::operator=(const address16& address)
{
    *this = address.get_value();
    return *this;
}

gameboy::register16& gameboy::register16::operator++()
{
    *this += 1;
    return *this;
}

gameboy::register16& gameboy::register16::operator--()
{
    *this -= 1;
    return *this;
}

gameboy::register16& gameboy::register16::operator+=(uint16_t val)
{
    *this = static_cast<uint16_t>(*this + val);
    return *this;
}

gameboy::register16& gameboy::register16::operator+=(const gameboy::register16& reg)
{
    *this += reg.value();
    return *this;
}

gameboy::register16& gameboy::register16::operator+=(const address8& address)
{
    *this = value() + static_cast<int8_t>(address.get_value());
    return *this;
}

gameboy::register16& gameboy::register16::operator-=(uint16_t val)
{
    *this = static_cast<uint16_t>(*this - val);
    return *this;
}

uint32_t gameboy::register16::operator+(const uint32_t val) const
{
    const uint32_t this_value = value();
    return this_value + val;
}

uint32_t gameboy::register16::operator+(const gameboy::register16& reg) const
{
    const uint32_t this_value = value();
    return this_value + reg.value();
}

uint32_t gameboy::register16::operator-(const uint32_t v) const
{
    const uint32_t this_value = value();
    return this_value - v;
}

gameboy::register16 gameboy::register16::operator&(uint16_t val) const
{
    return register16(value() & val);
}

gameboy::register16 gameboy::register16::operator|(uint16_t val) const
{
    return register16(value() | val);
}

gameboy::register16 gameboy::register16::operator^(uint16_t val) const
{
    return register16(value() ^ val);
}

gameboy::register16 gameboy::register16::operator~() const
{
    return register16(~value());
}
