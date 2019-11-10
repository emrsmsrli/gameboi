#ifndef GAMEBOY_GAMEBOY_H
#define GAMEBOY_GAMEBOY_H

#include <string_view>
#include <memory>

#include <bus.h>
#include <cartridge.h>
#include <cpu/cpu.h>
#include <ppu/ppu.h>
#include <memory/mmu.h>
#include <apu/apu.h>
#include <util/delegate.h>

namespace gameboy {

class gameboy {
    friend bus;

public:
    explicit gameboy(std::string_view rom_path);

    void start();
    void tick();

    [[nodiscard]] const std::string& rom_name() const noexcept { return cartridge_.name(); }

    void on_render_line(delegate<void(uint8_t, const render_line&)> delegate) noexcept
    {
        ppu_.on_render_line = delegate;
    }

    void on_render_frame(delegate<void()> delegate) noexcept
    {
        ppu_.on_render_frame = delegate;
    }

private:
    cartridge cartridge_;
    bus bus_;

    mmu mmu_;
    cpu cpu_;
    ppu ppu_;
    apu apu_;
};

} // namespace gameboy

#endif //GAMEBOY_GAMEBOY_H
