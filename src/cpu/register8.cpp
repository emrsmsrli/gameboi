#include <cpu/register8.h>

gameboy::register8& gameboy::register8::operator=(const uint8_t val)
{
    bits_ = val;
    return *this;
}

gameboy::register8& gameboy::register8::operator+=(const uint8_t val)
{
    bits_ += val;
    return *this;
}

gameboy::register8& gameboy::register8::operator-=(const uint8_t val)
{
    bits_ -= val;
    return *this;
}

uint16_t gameboy::register8::operator+(const uint16_t val) const
{
    return bits_ + val;
}

uint16_t gameboy::register8::operator-(const uint16_t val) const
{
    return bits_ - val;
}

gameboy::register8& gameboy::register8::operator&=(const uint8_t val)
{
    bits_ &= val;
    return *this;
}

gameboy::register8& gameboy::register8::operator|=(const uint8_t val)
{
    bits_ |= val;
    return *this;
}

gameboy::register8& gameboy::register8::operator^=(const uint8_t val)
{
    bits_ ^= val;
    return *this;
}

gameboy::register8 gameboy::register8::operator&(const uint8_t val) const
{
    return register8(bits_ & val);
}

gameboy::register8 gameboy::register8::operator|(const uint8_t val) const
{
    return register8(bits_ | val);
}

gameboy::register8 gameboy::register8::operator^(const uint8_t val) const
{
    return register8(bits_ ^ val);
}

gameboy::register8 gameboy::register8::operator~() const
{
    return register8(~bits_);
}

bool gameboy::register8::operator==(const uint8_t val) const
{
    return bits_ == val;
}

bool gameboy::register8::operator>(const uint8_t val) const
{
    return bits_ > val;
}

bool gameboy::register8::operator<(const uint8_t val) const
{
    return bits_ < val;
}
