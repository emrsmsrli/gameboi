#ifndef GAMEBOY_GAMEBOY_H
#define GAMEBOY_GAMEBOY_H

#include "gameboy/bus.h"
#include "gameboy/cartridge.h"
#include "gameboy/cpu/cpu.h"
#include "gameboy/ppu/ppu.h"
#include "gameboy/memory/mmu.h"
#include "gameboy/apu/apu.h"
#include "gameboy/joypad/joypad.h"
#include "gameboy/timer/timer.h"
#include "gameboy/util/delegate.h"
#include "gameboy/link/link.h"

namespace gameboy {

class gameboy {
    friend bus;

public:
    explicit gameboy(std::string_view rom_path);

    void start();
    void tick();
    void tick_one_frame();

    [[nodiscard]] const std::string& rom_name() const noexcept { return cartridge_.name(); }

    void on_render_line(const ppu::render_line_func on_render_line) noexcept { ppu_.on_render_line(on_render_line); }
    void on_vblank(const ppu::vblank_func on_vblank) noexcept { ppu_.on_vblank(on_vblank); }
    void on_link_transfer(const link::transfer_func on_transfer) { link_.on_transfer(on_transfer); }

    void press_key(const joypad::key key) noexcept { joypad_.press(key); }
    void release_key(const joypad::key key) noexcept { joypad_.release(key); }

    [[nodiscard]] observer<bus> get_bus() { return make_observer(bus_); }

private:
    cartridge cartridge_;
    bus bus_;

    mmu mmu_;
    cpu cpu_;
    ppu ppu_;
    apu apu_;
    link link_;
    joypad joypad_;
    timer timer_;
};

} // namespace gameboy

#endif //GAMEBOY_GAMEBOY_H
