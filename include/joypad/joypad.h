#ifndef GAMEBOY_JOYPAD_H
#define GAMEBOY_JOYPAD_H

#include <cpu/register8.h>
#include <memory/addressfwd.h>
#include <util/observer.h>
#include <util/enumutil.h>

namespace gameboy {

class bus;

/*uint8_t hi = (reg_ & 0xF0);

if ((hi & 0x30) == 0x10 || (hi & 0x30) == 0x20)
{
    // first 2 bits of high nybble is group selection
    uint8_t group = ((~(reg_ >> 4)) & 0x03) - 1;

    uint8_t selection = (keys_ >> (group * 4)) & 0x0F;

    return (reg_ & 0xF0) | selection;
}
else
{
    return reg_ | 0x0F;
}*/
/**
 * Bit 5 - P15 Select Button Keys      (0=Select)
 * Bit 4 - P14 Select Direction Keys   (0=Select)
 * Bit 3 - P13 Input Down  or Start    (0=Pressed) (Read Only)
 * Bit 2 - P12 Input Up    or Select   (0=Pressed) (Read Only)
 * Bit 1 - P11 Input Left  or Button B (0=Pressed) (Read Only)
 * Bit 0 - P10 Input Right or Button A (0=Pressed) (Read Only)
 *            P14     P15
 *      --------------------------
 *  P10 |    right    a        |
 *  P11 |    left     b        |
 *  P12 |    up       select   |
 *  P13 |    down     start    |
 *      --------------------------
 */
class joypad {
public:
    enum class key : uint8_t {
        none = 0u,
        right = 1u << 0u,
        left = 1u << 1u,
        up = 1u << 2u,
        down = 1u << 3u,
        a = 1u << 4u,
        b = 1u << 5u,
        select = 1u << 6u,
        start = 1u << 7u
    };

    explicit joypad(observer<bus> bus);

    void press(key key) noexcept;
    void release(key key) noexcept;

    [[nodiscard]] uint8_t read(const address16&) const noexcept;
    void write(const address16&, uint8_t data) noexcept;

private:
    observer<bus> bus_;

    register8 joyp_;
    key keys_;
};

DEFINE_ENUM_CLASS_FLAGS(joypad::key)

} // namespace gameboy

#endif //GAMEBOY_JOYPAD_H
