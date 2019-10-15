#include <cpu/register16.h>
#include <memory/address.h>

namespace gameboy {

uint16_t register16::value() const
{
    const uint16_t h = high_.value();
    return (h << 8u) | low_.value();
}

register16& register16::operator=(uint16_t val)
{
    low_ = val & 0xFFu;
    high_ = (val >> 0x8u) & 0xFFu;
    return *this;
}

register16& register16::operator=(const address16& address)
{
    *this = address.value();
    return *this;
}

register16& register16::operator++()
{
    *this += 1;
    return *this;
}

register16& register16::operator--()
{
    *this -= 1;
    return *this;
}

register16& register16::operator+=(uint16_t val)
{
    *this = static_cast<uint16_t>(*this + val);
    return *this;
}

register16& register16::operator+=(const register16& reg)
{
    *this += reg.value();
    return *this;
}

register16& register16::operator+=(const address8& address)
{
    *this = value() + static_cast<int8_t>(address.value());
    return *this;
}

register16& register16::operator-=(uint16_t val)
{
    *this = static_cast<uint16_t>(*this - val);
    return *this;
}

uint32_t register16::operator+(const uint32_t val) const
{
    const uint32_t this_value = value();
    return this_value + val;
}

uint32_t register16::operator+(const register16& reg) const
{
    const uint32_t this_value = value();
    return this_value + reg.value();
}

uint32_t register16::operator-(const uint32_t v) const
{
    const uint32_t this_value = value();
    return this_value - v;
}

register16 register16::operator&(uint16_t val) const
{
    return register16(value() & val);
}

register16 register16::operator|(uint16_t val) const
{
    return register16(value() | val);
}

register16 register16::operator^(uint16_t val) const
{
    return register16(value() ^ val);
}

register16 register16::operator~() const
{
    return register16(~value());
}

} // namespace gameboy
