#ifndef GAMEBOY_GAMEBOY_H
#define GAMEBOY_GAMEBOY_H

#include <string_view>
#include <memory>
#include "cpu/CPU.h"
#include "memory/MMU.h"

namespace gameboy {
    class GameBoy {
    public:
        explicit GameBoy(std::string_view cartridge_path);

        void start();

    private:
        std::unique_ptr<memory::MMU> memory;
        std::unique_ptr<cpu::CPU> cpu;
    };
}

#endif //GAMEBOY_GAMEBOY_H
