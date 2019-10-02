#include <ppu/PPU.h>

gameboy::ppu::PPU::PPU(std::shared_ptr<memory::MMU> memory_management_unit)
        : mmu(std::move(memory_management_unit)) { }

void gameboy::ppu::PPU::tick()
{

}
