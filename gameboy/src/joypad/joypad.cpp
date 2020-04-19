#include <magic_enum.hpp>

#include "gameboy/joypad/joypad.h"
#include "gameboy/bus.h"
#include "gameboy/cpu/cpu.h"
#include "gameboy/memory/mmu.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

using namespace magic_enum::bitwise_operators;

constexpr address16 joypad_addr{0xFF00u};

joypad::joypad(observer<bus> bus)
    : bus_{bus},
      joyp_{0x0Fu},
      keys_{0xFFu}
{
    bus->get_mmu()->add_memory_delegate(joypad_addr, {
        {connect_arg<&joypad::read>, this},
        {connect_arg<&joypad::write>, this}
    });
}

void joypad::press(const key key) noexcept
{
    keys_ &= ~key;
    bus_->get_cpu()->request_interrupt(interrupt::joypad);
}

void joypad::release(const key key) noexcept
{
    keys_ |= key;
}

uint8_t joypad::read(const address16&) const noexcept
{
    if(const auto select = joyp_ & 0x30u; !mask::test(select, 0x10u)) {
        return ((joyp_ & 0xF0u) | (static_cast<uint8_t>(keys_) & 0x0Fu)).value();
    } else if(!mask::test(select, 0x20u)) {
        return ((joyp_ & 0xF0u) | (static_cast<uint8_t>(keys_) >> 4u)).value();
    }

    return joyp_.value();
}

void joypad::write(const address16&, const uint8_t data) noexcept
{
    joyp_ = data | 0xCFu;
}

} // namespace gameboy
