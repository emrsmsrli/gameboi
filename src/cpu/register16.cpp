#include <cpu/register16.h>
#include <memory/address.h>

uint16_t gameboy::register16::get_value() const
{
    const uint16_t value = high.get_value();
    return (value << 8u) | low.get_value();
}

gameboy::register16& gameboy::register16::operator=(uint16_t value)
{
    low = value & 0xFFu;
    high = (value >> 0x8u) & 0xFFu;
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

gameboy::register16& gameboy::register16::operator+=(uint16_t value)
{
    *this = static_cast<uint16_t>(*this + value);
    return *this;
}

gameboy::register16& gameboy::register16::operator+=(const gameboy::register16& reg)
{
    *this += reg.get_value();
    return *this;
}

gameboy::register16& gameboy::register16::operator+=(const address16& address)
{
    *this += address.get_value();
    return *this;
}

gameboy::register16& gameboy::register16::operator-=(uint16_t value)
{
    *this = static_cast<uint16_t>(*this - value);
    return *this;
}

gameboy::register16& gameboy::register16::operator-=(const gameboy::register16& reg)
{
    *this -= reg.get_value();
    return *this;
}

gameboy::register16& gameboy::register16::operator-=(const address16& address)
{
    *this -= address.get_value();
    return *this;
}

uint32_t gameboy::register16::operator+(const uint32_t value) const
{
    const uint32_t this_value = get_value();
    return this_value + value;
}

uint32_t gameboy::register16::operator+(const gameboy::register16& reg) const
{
    const uint32_t this_value = get_value();
    return this_value + reg.get_value();
}

uint32_t gameboy::register16::operator+(const address16& address) const
{
    const uint32_t this_value = get_value();
    return this_value + address.get_value();
}

uint32_t gameboy::register16::operator-(const uint32_t value) const
{
    const uint32_t this_value = get_value();
    return this_value - value;
}

uint32_t gameboy::register16::operator-(const gameboy::register16& reg) const
{
    const uint32_t this_value = get_value();
    return this_value - reg.get_value();
}

uint32_t gameboy::register16::operator-(const address16& address) const
{
    const uint32_t this_value = get_value();
    return this_value - address.get_value();
}

gameboy::register16& gameboy::register16::operator&=(uint16_t value)
{
    *this = get_value() & value;
    return *this;
}

gameboy::register16& gameboy::register16::operator&=(const gameboy::register16& reg)
{
    *this &= reg.get_value();
    return *this;
}

gameboy::register16& gameboy::register16::operator|=(uint16_t value)
{
    *this = get_value() | value;
    return *this;
}

gameboy::register16& gameboy::register16::operator|=(const gameboy::register16& reg)
{
    *this |= reg.get_value();
    return *this;
}

gameboy::register16& gameboy::register16::operator^=(uint16_t value)
{
    *this = get_value() ^ value;
    return *this;
}

gameboy::register16& gameboy::register16::operator^=(const gameboy::register16& reg)
{
    *this ^= reg.get_value();
    return *this;
}

gameboy::register16 gameboy::register16::operator&(uint16_t value) const
{
    return register16(get_value() & value);
}

gameboy::register16 gameboy::register16::operator&(const gameboy::register16& reg) const
{
    return register16(get_value() & reg.get_value());
}

gameboy::register16 gameboy::register16::operator|(uint16_t value) const
{
    return register16(get_value() | value);
}

gameboy::register16 gameboy::register16::operator|(const gameboy::register16& reg) const
{
    return register16(get_value() | reg.get_value());
}

gameboy::register16 gameboy::register16::operator^(uint16_t value) const
{
    return register16(get_value() ^ value);
}

gameboy::register16 gameboy::register16::operator^(const gameboy::register16& reg) const
{
    return register16(get_value() ^ reg.get_value());
}

gameboy::register16 gameboy::register16::operator~() const
{
    return register16(~get_value());
}

bool gameboy::register16::operator==(uint16_t value) const
{
    return get_value() == value;
}

bool gameboy::register16::operator==(const gameboy::register16& reg) const
{
    return get_value() == reg.get_value();
}

bool gameboy::register16::operator==(const address16& address) const
{
    return get_value() == address.get_value();
}

bool gameboy::register16::operator!=(uint16_t value) const
{
    return get_value() != value;
}

bool gameboy::register16::operator!=(const gameboy::register16& reg) const
{
    return get_value() != reg.get_value();
}

bool gameboy::register16::operator!=(const address16& address) const
{
    return get_value() != address.get_value();
}

bool gameboy::register16::operator>(uint16_t value) const
{
    return get_value() > value;
}

bool gameboy::register16::operator>(const gameboy::register16& reg) const
{
    return get_value() > reg.get_value();
}

bool gameboy::register16::operator>(const address16& address) const
{
    return get_value() > address.get_value();
}

bool gameboy::register16::operator<(uint16_t value) const
{
    return get_value() < value;
}

bool gameboy::register16::operator<(const gameboy::register16& reg) const
{
    return get_value() < reg.get_value();
}

bool gameboy::register16::operator<(const address16& address) const
{
    return get_value() < address.get_value();
}

bool gameboy::register16::operator>=(uint16_t value) const
{
    return get_value() >= value;
}

bool gameboy::register16::operator>=(const gameboy::register16& reg) const
{
    return get_value() >= reg.get_value();
}

bool gameboy::register16::operator>=(const address16& address) const
{
    return get_value() >= address.get_value();
}

bool gameboy::register16::operator<=(uint16_t value) const
{
    return get_value() <= value;
}

bool gameboy::register16::operator<=(const gameboy::register16& reg) const
{
    return get_value() <= reg.get_value();
}

bool gameboy::register16::operator<=(const address16& address) const
{
    return get_value() <= address.get_value();
}
