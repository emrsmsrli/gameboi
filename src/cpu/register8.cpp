#include <cpu/register8.h>
#include <memory/address.h>

gameboy::register8& gameboy::register8::operator=(const uint8_t value)
{
    bits = value;
    return *this;
}

gameboy::register8& gameboy::register8::operator=(const gameboy::address8& address)
{
    bits = address.get_value();
    return *this;
}

gameboy::register8& gameboy::register8::operator++()
{
    ++bits;
    return *this;
}

gameboy::register8& gameboy::register8::operator--()
{
    --bits;
    return *this;
}

gameboy::register8& gameboy::register8::operator+=(const uint8_t value)
{
    bits += value;
    return *this;
}

gameboy::register8& gameboy::register8::operator+=(const gameboy::register8& reg)
{
    bits += reg.bits;
    return *this;
}

gameboy::register8& gameboy::register8::operator+=(const gameboy::address8& address)
{
    bits += address.get_value();
    return *this;
}

gameboy::register8& gameboy::register8::operator-=(const uint8_t value)
{
    bits -= value;
    return *this;
}

gameboy::register8& gameboy::register8::operator-=(const gameboy::register8& reg)
{
    bits -= reg.bits;
    return *this;
}

gameboy::register8& gameboy::register8::operator-=(const gameboy::address8& address)
{
    bits -= address.get_value();
    return *this;
}

auto gameboy::register8::operator+(uint16_t value) const -> uint16_t
{
    return bits + value;
}

uint16_t gameboy::register8::operator+(const gameboy::register8& reg) const
{
    return static_cast<uint16_t>(bits) + reg.bits;
}

uint16_t gameboy::register8::operator+(const address8& address) const
{
    return static_cast<uint16_t>(bits) + address.get_value();
}

uint16_t gameboy::register8::operator-(const uint16_t value) const
{
    return bits - value;
}

uint16_t gameboy::register8::operator-(const gameboy::register8& reg) const
{
    return static_cast<uint16_t>(bits) - reg.bits;
}

uint16_t gameboy::register8::operator-(const address8& address) const
{
    return static_cast<uint16_t>(bits) - address.get_value();
}

gameboy::register8& gameboy::register8::operator&=(const uint8_t value)
{
    bits &= value;
    return *this;
}

gameboy::register8& gameboy::register8::operator&=(const gameboy::register8& reg)
{
    bits &= reg.bits;
    return *this;
}

gameboy::register8& gameboy::register8::operator|=(const uint8_t value)
{
    bits |= value;
    return *this;
}

gameboy::register8& gameboy::register8::operator|=(const gameboy::register8& reg)
{
    bits |= reg.bits;
    return *this;
}

gameboy::register8& gameboy::register8::operator^=(const uint8_t value)
{
    bits ^= value;
    return *this;
}

gameboy::register8& gameboy::register8::operator^=(const gameboy::register8& reg)
{
    bits ^= reg.bits;
    return *this;
}

gameboy::register8 gameboy::register8::operator&(const uint8_t value) const
{
    return register8(bits & value);
}

gameboy::register8 gameboy::register8::operator&(const gameboy::register8& reg) const
{
    return register8(bits & reg.bits);
}

gameboy::register8 gameboy::register8::operator|(const uint8_t value) const
{
    return register8(bits | value);
}

gameboy::register8 gameboy::register8::operator|(const gameboy::register8& reg) const
{
    return register8(bits | reg.bits);
}

gameboy::register8 gameboy::register8::operator^(const uint8_t value) const
{
    return register8(bits ^ value);
}

gameboy::register8 gameboy::register8::operator^(const gameboy::register8& reg) const
{
    return register8(bits ^ reg.bits);
}

gameboy::register8 gameboy::register8::operator~() const
{
    return register8(~bits);
}

bool gameboy::register8::operator==(const uint8_t value) const
{
    return bits == value;
}

bool gameboy::register8::operator==(const gameboy::register8& reg) const
{
    return bits == reg.bits;
}

bool gameboy::register8::operator==(const address8& address) const
{
    return bits == address.get_value();
}

bool gameboy::register8::operator!=(const uint8_t value) const
{
    return bits != value;
}

bool gameboy::register8::operator!=(const gameboy::register8& reg) const
{
    return bits != reg.bits;
}

bool gameboy::register8::operator!=(const address8& address) const
{
    return bits != address.get_value();
}

bool gameboy::register8::operator>(const uint8_t value) const
{
    return bits > value;
}

bool gameboy::register8::operator>(const gameboy::register8& reg) const
{
    return bits > reg.bits;
}

bool gameboy::register8::operator>(const address8& address) const
{
    return bits > address.get_value();
}

bool gameboy::register8::operator<(const uint8_t value) const
{
    return bits < value;
}

bool gameboy::register8::operator<(const gameboy::register8& reg) const
{
    return bits < reg.bits;
}

bool gameboy::register8::operator<(const address8& address) const
{
    return bits < address.get_value();
}

bool gameboy::register8::operator>=(const uint8_t value) const
{
    return bits >= value;
}

bool gameboy::register8::operator>=(const gameboy::register8& reg) const
{
    return bits >= reg.bits;
}

bool gameboy::register8::operator>=(const address8& address) const
{
    return bits >= address.get_value();
}

bool gameboy::register8::operator<=(const uint8_t value) const
{
    return bits <= value;
}

bool gameboy::register8::operator<=(const gameboy::register8& reg) const
{
    return bits <= reg.bits;
}

bool gameboy::register8::operator<=(const address8& address) const
{
    return bits <= address.get_value();
}
