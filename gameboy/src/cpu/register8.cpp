#include "gameboy/cpu/register8.h"
#include "gameboy/memory/address.h"

namespace gameboy {

register8& register8::operator=(const uint8_t val) noexcept
{
    bits_ = val;
    return *this;
}

register8& register8::operator=(const address8& val) noexcept
{
    bits_ = val.value();
    return *this;
}

register8& register8::operator+=(const uint8_t val) noexcept
{
    bits_ += val;
    return *this;
}

register8& register8::operator-=(const uint8_t val) noexcept
{
    bits_ -= val;
    return *this;
}

register8 register8::operator+(const register8& val) const noexcept
{
    return register8(bits_ + val.value());
}

uint16_t register8::operator+(const uint16_t val) const noexcept
{
    return bits_ + val;
}

uint16_t register8::operator-(const uint16_t val) const noexcept
{
    return bits_ - val;
}

register8& register8::operator&=(const uint8_t val) noexcept
{
    bits_ &= val;
    return *this;
}

register8& register8::operator|=(const uint8_t val) noexcept
{
    bits_ |= val;
    return *this;
}

register8& register8::operator^=(const uint8_t val) noexcept
{
    bits_ ^= val;
    return *this;
}

register8 register8::operator&(const uint8_t val) const noexcept
{
    return register8(bits_ & val);
}

register8 register8::operator|(const uint8_t val) const noexcept
{
    return register8(bits_ | val);
}

register8 register8::operator^(const uint8_t val) const noexcept
{
    return register8(bits_ ^ val);
}

register8 register8::operator~() const noexcept
{
    return register8(~bits_);
}

bool register8::operator==(const uint8_t val) const noexcept
{
    return bits_ == val;
}

bool register8::operator==(const register8& other) const noexcept
{
    return bits_ == other.bits_;
}

bool register8::operator>(const uint8_t val) const noexcept
{
    return bits_ > val;
}

bool register8::operator<(const uint8_t val) const noexcept
{
    return bits_ < val;
}

bool register8::operator>=(const uint8_t val) const noexcept
{
    return bits_ >= val;
}

bool register8::operator>=(const register8& other) const noexcept
{
    return *this >= other.value();
}

bool register8::operator<=(const uint8_t val) const noexcept
{
    return bits_ <= val;
}

bool register8::operator<=(const register8& other) const noexcept
{
    return *this <= other.value();
}

} // namespace gameboy
