#ifndef GAMEBOY_PPU_H
#define GAMEBOY_PPU_H

#include <memory>
#include <memory/MMU.h>

namespace gameboy::ppu {
    class PPU {
    public:
        explicit PPU(std::shared_ptr<memory::MMU> memory_management_unit);

        void tick();

    private:
        std::shared_ptr<memory::MMU> mmu;
    };
}

#endif //GAMEBOY_PPU_H
