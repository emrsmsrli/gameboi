#ifndef GAMEBOY_GAMEBOY_H
#define GAMEBOY_GAMEBOY_H

#include <string_view>
#include <memory>
#include <cpu/CPU.h>
#include <ppu/PPU.h>
#include <memory/MMU.h>

namespace gameboy {
    class GameBoy {
    public:
        explicit GameBoy(std::string_view rom_path);

        void start();

    private:
        std::shared_ptr<memory::MMU> memory;
        std::unique_ptr<cpu::CPU> cpu;
        std::unique_ptr<ppu::PPU> ppu;
    };
}

#endif //GAMEBOY_GAMEBOY_H
