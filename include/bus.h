#ifndef GAMEBOY_BUS_H
#define GAMEBOY_BUS_H

#include <util/observer.h>

namespace gameboy {

class cartridge;
class cpu;
class ppu;
class apu;

class bus {
public:
    explicit bus(observer<cartridge> cartridge)
        : cartridge(cartridge) {}

    observer<cartridge> cartridge;
    observer<cpu> cpu;
    observer<ppu> ppu;
    observer<apu> apu;
};

}

#endif //GAMEBOY_BUS_H
