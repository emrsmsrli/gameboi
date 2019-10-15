#include <cpu/register8.h>

namespace gameboy {

register8& register8::operator=(const uint8_t val) noexcept
{
    bits_ = val;
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

bool register8::operator>(const uint8_t val) const noexcept
{
    return bits_ > val;
}

bool register8::operator<(const uint8_t val) const noexcept
{
    return bits_ < val;
}

} // namespace gameboy
