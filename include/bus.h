#ifndef GAMEBOY_BUS_H
#define GAMEBOY_BUS_H

#include <util/observer.h>
#include <cpu/cpu.h>
#include <ppu/ppu.h>

namespace gameboy {
    class bus {
    public:
        bus() = default;

    private:
        observer<cpu> cpu_;
        observer<ppu> ppu_;
        observer<cpu> apu_;
    };
}

#endif //GAMEBOY_BUS_H
