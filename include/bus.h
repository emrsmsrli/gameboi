#ifndef GAMEBOY_BUS_H
#define GAMEBOY_BUS_H

#include <util/observer.h>

namespace gameboy {

class cartridge;
class mmu;
class cpu;
class ppu;
class apu;

struct bus {
    explicit bus(observer<cartridge> cartridge) noexcept
        : cartridge(cartridge) {}

    observer<cartridge> cartridge;
    observer<mmu> mmu;
    observer<cpu> cpu;
    observer<ppu> ppu;
    observer<apu> apu;
};

}

#endif //GAMEBOY_BUS_H
