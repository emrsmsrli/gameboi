#ifndef GAMEBOY_JOYPAD_H
#define GAMEBOY_JOYPAD_H

#include "gameboy/cpu/register8.h"
#include "gameboy/memory/addressfwd.h"
#include "gameboy/util/observer.h"

namespace gameboy {

class bus;
class joypad_debugger;

class joypad {
    friend joypad_debugger;

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

} // namespace gameboy

#endif //GAMEBOY_JOYPAD_H
