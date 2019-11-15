#ifndef GAMEBOY_JOYPAD_H
#define GAMEBOY_JOYPAD_H

#include <cpu/register8.h>
#include <memory/addressfwd.h>
#include <util/observer.h>
#include <util/enumutil.h>

namespace gameboy {

class bus;

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

DEFINE_ENUM_CLASS_FLAGS(joypad::key);

} // namespace gameboy

#endif //GAMEBOY_JOYPAD_H
