#ifndef GAMEBOY_GAMEBOY_H
#define GAMEBOY_GAMEBOY_H

#include <string_view>
#include <memory>

#include <cartridge.h>
#include <cpu/cpu.h>
#include <ppu/ppu.h>
#include <memory/mmu.h>

namespace gameboy {

class gameboy {
public:
    explicit gameboy(std::string_view rom_path);

    void start();

private:
    cartridge cartridge_;

    std::shared_ptr<mmu> memory_;
    std::unique_ptr<cpu> cpu_;
    std::unique_ptr<ppu> ppu_;
};

}

#endif //GAMEBOY_GAMEBOY_H
