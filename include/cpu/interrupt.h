#ifndef GAMEBOY_INTERRUPT_H
#define GAMEBOY_INTERRUPT_H

#include <cstdint>

#include <memory/address.h>
#include <util/enumutil.h>

namespace gameboy {

enum class interrupt : uint8_t {
    none = 0u,
    lcd_vblank = 0x1u,
    lcd_stat = 0x2u,
    timer = 0x4u,
    serial = 0x8u,
    joypad = 0x16u,
    all = lcd_vblank | lcd_stat | timer | serial | joypad
};

DEFINE_ENUM_CLASS_FLAGS(interrupt);

constexpr address8 make_address(const interrupt interrupt_request)
{
    switch(interrupt_request) {
        case interrupt::lcd_vblank:
            return address8{0x40u};
        case interrupt::lcd_stat:
            return address8{0x48u};
        case interrupt::timer:
            return address8{0x50u};
        case interrupt::serial:
            return address8{0x58u};
        case interrupt::joypad:
            return address8{0x60u};
        case interrupt::all:
        case interrupt::none:
            return address8{0u};
    }
}

} // namespace gameboy

#endif //GAMEBOY_INTERRUPT_H
