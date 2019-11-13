#include <joypad/joypad.h>
#include <bus.h>
#include <cpu/cpu.h>
#include <memory/mmu.h>

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

/*
 * The eight gameboy buttons/direction keys are arranged in form of a 2x4 matrix.
 * Select either button or direction keys by writing to this register, then read-out bit 0-3.
 *
 *   Bit 7 - Not used
 *   Bit 6 - Not used
 *   Bit 5 - P15 Select Button Keys      (0=Select)
 *   Bit 4 - P14 Select Direction Keys   (0=Select)
 *   Bit 3 - P13 Input Down  or Start    (0=Pressed) (Read Only)
 *   Bit 2 - P12 Input Up    or Select   (0=Pressed) (Read Only)
 *   Bit 1 - P11 Input Left  or Button B (0=Pressed) (Read Only)
 *   Bit 0 - P10 Input Right or Button A (0=Pressed) (Read Only)
 */
uint8_t joypad::read(const address16&) const noexcept
{
    if(const auto select = joyp_ & 0xF0u; (select & 0x30u) == 0x10u || (select & 0x30u) == 0x20u) {
        const auto group = ((~(joyp_.value() >> 4u)) & 0x03u) - 1u;
        const auto selection = (static_cast<uint8_t>(keys_) >> (group * 4u)) & 0x0Fu;
        return (joyp_.value() & 0xF0u) | selection;
    } else {
        return joyp_.value() | 0x0Fu;
    }
}

void joypad::write(const address16&, const uint8_t data) noexcept
{
    joyp_ = data;
}

} // namespace gameboy
