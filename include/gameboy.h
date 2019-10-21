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

namespace gameboy {

class gameboy {
public:
    explicit gameboy(std::string_view rom_path);

    void start();
    void tick();

private:
    cartridge cartridge_;
    bus bus_;

    mmu mmu_;
    cpu cpu_;
    ppu ppu_;
    apu apu_;
};

}

#endif //GAMEBOY_GAMEBOY_H
