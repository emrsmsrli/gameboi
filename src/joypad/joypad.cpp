#include <joypad/joypad.h>
#include <bus.h>
#include <cpu/cpu.h>
#include <memory/mmu.h>
#include <util/mathutil.h>

namespace gameboy {

constexpr address16 joypad_addr(0xFF00u);

joypad::joypad(observer<bus> bus)
    : bus_(bus),
      keys_{0xFFu}
{
    bus->get_mmu()->add_memory_callback({
        joypad_addr,
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
    if(const auto select = joyp_ & 0xF0u; mask_test(select, 0x10u) || mask_test(select, 0x20u)) {
        const auto group = mask(~(select.value() >> 4u), 0x03u) - 1u;
        const auto selection = mask(static_cast<uint8_t>(keys_) >> (group * 4u), 0x0Fu);
        return mask_set(mask(joyp_, 0xF0u), selection).value();
    } else {
        return mask_set(joyp_, 0x0Fu).value();
    }
}

void joypad::write(const address16&, const uint8_t data) noexcept
{
    joyp_ = data;
}

} // namespace gameboy
