#include "gameboy/cpu/register16.h"

#include "gameboy/memory/address.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

uint16_t register16::value() const noexcept
{
    return word(high_.value(), low_.value());
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

register16& register16::operator+=(const address8& address) noexcept
{
    *this = static_cast<uint16_t>(value() + static_cast<int8_t>(address.value()));
    return *this;
}

} // namespace gameboy
