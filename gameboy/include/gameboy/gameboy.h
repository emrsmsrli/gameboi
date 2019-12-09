#ifndef GAMEBOY_GAMEBOY_H
#define GAMEBOY_GAMEBOY_H

#include <string_view>

#include "gameboy/bus.h"
#include "gameboy/cartridge.h"
#include "gameboy/cpu/cpu.h"
#include "gameboy/ppu/ppu.h"
#include "gameboy/memory/mmu.h"
#include "gameboy/apu/apu.h"
#include "gameboy/joypad/joypad.h"
#include "gameboy/util/delegate.h"

namespace gameboy {

class gameboy {
    friend bus;

public:
    explicit gameboy(std::string_view rom_path);

    void start();
    void tick();

    [[nodiscard]] const std::string& rom_name() const noexcept { return cartridge_.name(); }

    void on_render_line(const delegate<void(uint8_t, const render_line&)> delegate) noexcept
    {
        ppu_.on_render_line = delegate;
    }

    void on_render_frame(const delegate<void()> delegate) noexcept
    {
        ppu_.on_render_frame = delegate;
    }

    void press_key(const joypad::key key) noexcept { joypad_.press(key); }
    void release_key(const joypad::key key) noexcept { joypad_.release(key); }

private:
    cartridge cartridge_;
    bus bus_;

    mmu mmu_;
    cpu cpu_;
    ppu ppu_;
    apu apu_;
    joypad joypad_;
};

} // namespace gameboy

#endif //GAMEBOY_GAMEBOY_H
