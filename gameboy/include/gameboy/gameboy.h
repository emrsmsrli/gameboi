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
#include "gameboy/link/link.h"
#include "gameboy/util/delegate.h"
#include "gameboy/util/fileutil.h"

namespace gameboy {

class gameboy {
    friend bus;

public:
#if WITH_DEBUGGER
    bool tick_enabled = false;
#endif //WITH_DEBUGGER

    explicit gameboy(const filesystem::path& rom_path);

    void tick();
    void tick_one_frame();

    [[nodiscard]] const std::string& rom_name() const noexcept { return cartridge_.name(); }

    void on_render_line(const ppu::render_line_func on_render_line) noexcept { ppu_.on_render_line(on_render_line); }
    void on_vblank(const ppu::vblank_func on_vblank) noexcept { ppu_.on_vblank(on_vblank); }
    void set_gb_palette(const palette& palette) noexcept { ppu_.set_gb_palette(palette); }

    [[maybe_unused]] void on_link_transfer_master(const link::transfer_func on_transfer) noexcept { link_.on_transfer_master(on_transfer); }
    [[maybe_unused]] [[nodiscard]] uint8_t on_link_transfer_slave(const uint8_t data) noexcept { return link_.on_transfer_slave(data); }

    void on_audio_buffer_full(const apu::sound_buffer_full_func on_buffer_full) noexcept { apu_.on_sound_buffer_full(on_buffer_full); }

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
