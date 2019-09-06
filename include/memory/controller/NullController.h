#ifndef GAMEBOY_NULLCONTROLLER_H
#define GAMEBOY_NULLCONTROLLER_H

#include <memory/controller/MBC.h>

/**
 * Rom only
 */
namespace gameboy::memory::controller {
    class NullController : public MBC {
    public:
        NullController(const std::vector<uint8_t>& rom, const CartridgeInfo& rom_header)
                : MBC(rom, rom_header) { }

    protected:
        void select_rom_bank(uint8_t data) override {}
        void control(const Address16&, uint8_t) override {}
    };
}

#endif //GAMEBOY_NULLCONTROLLER_H
