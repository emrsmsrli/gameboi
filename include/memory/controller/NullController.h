//
// Created by Emre Şimşirli on 25.08.2019.
//

#ifndef GAMEBOY_NULLCONTROLLER_H
#define GAMEBOY_NULLCONTROLLER_H

#include "MBC.h"

/**
 * Rom only
 */
namespace gameboy::memory::controller {
    class NullController : public MBC {
    public:
        NullController(const std::vector<uint8_t>& rom, const CartridgeInfo& rom_header)
                : MBC(rom, rom_header) { }

    protected:
        void control(const Address16&, uint8_t) override {}
    };
}

#endif //GAMEBOY_NULLCONTROLLER_H
