#include "gameboy/cpu/register16.h"
#include "gameboy/memory/address.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

uint16_t register16::value() const noexcept
{
    return word(high_.value(), low_.value());
}

register16& register16::operator=(const uint16_t val) noexcept
{
    low_ = val & 0xFFu;
    high_ = val >> 0x8u & 0xFFu;
    return *this;
}

register16& register16::operator=(const address8& address) noexcept
{
    low_ = address;
    high_ = 0x00u;
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

register16 register16::operator++(int) noexcept
{
    const auto copy = *this;
    *this += 1;
    return copy;
}

register16& register16::operator--() noexcept
{
    *this -= 1;
    return *this;
}

register16 register16::operator--(int) noexcept
{
    const auto copy = *this;
    *this -= 1;
    return copy;
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
    *this = static_cast<uint16_t>(value() + static_cast<int8_t>(address.value()));
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

bool register16::operator==(const uint16_t val) const noexcept
{
    return value() == val;
}

bool register16::operator==(const register16& other) const noexcept
{
    return value() == other.value();
}

} // namespace gameboy
