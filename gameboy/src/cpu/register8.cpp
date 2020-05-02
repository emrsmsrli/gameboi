#include "gameboy/cpu/register8.h"

#include "gameboy/memory/address.h"

namespace gameboy {

register8& register8::operator=(const address8& val) noexcept
{
    bits_ = val.value();
    return *this;
}

} // namespace gameboy
