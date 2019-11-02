#include <cpu/register16.h>
#include <memory/address.h>

namespace gameboy {

uint16_t register16::value() const noexcept
{
    const uint16_t h = high_.value();
    return h << 8u | low_.value();
}

register16& register16::operator=(const uint16_t val) noexcept
{
    low_ = val & 0xFFu;
    high_ = val >> 0x8u & 0xFFu;
    return *this;
}

register16& register16::operator=(const address16& address) noexcept
{
    *this = address.value();
    return *this;
}

register16& register16::operator++() noexcept
{
    *this += 1;
    return *this;
}

register16& register16::operator--() noexcept
{
    *this -= 1;
    return *this;
}

register16& register16::operator+=(const uint16_t val) noexcept
{
    *this = static_cast<uint16_t>(*this + val);
    return *this;
}

register16& register16::operator+=(const register16& reg) noexcept
{
    *this += reg.value();
    return *this;
}

register16& register16::operator+=(const address8& address) noexcept
{
    *this = value() + static_cast<int8_t>(address.value());
    return *this;
}

register16& register16::operator-=(const uint16_t val) noexcept
{
    *this = static_cast<uint16_t>(*this - val);
    return *this;
}

uint32_t register16::operator+(const uint32_t val) const noexcept
{
    const uint32_t this_value = value();
    return this_value + val;
}

uint32_t register16::operator+(const register16& reg) const noexcept
{
    const uint32_t this_value = value();
    return this_value + reg.value();
}

uint32_t register16::operator-(const uint32_t val) const noexcept
{
    const uint32_t this_value = value();
    return this_value - val;
}

register16 register16::operator&(const uint16_t val) const noexcept
{
    return register16(value() & val);
}

register16 register16::operator|(const uint16_t val) const noexcept
{
    return register16(value() | val);
}

register16 register16::operator^(const uint16_t val) const noexcept
{
    return register16(value() ^ val);
}

register16 register16::operator~() const noexcept
{
    return register16(~value());
}

} // namespace gameboy
