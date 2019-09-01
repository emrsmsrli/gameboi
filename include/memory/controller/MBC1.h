//
// Created by Emre Şimşirli on 25.08.2019.
//

#ifndef GAMEBOY_MBC1_H
#define GAMEBOY_MBC1_H

#include <cstdint>
#include "MBC.h"

namespace gameboy::memory::controller {
    class MBC1 : public MBC {
    public:
        MBC1(const std::vector<uint8_t>& rom, const CartridgeInfo& rom_header);

    protected:
        void control(const Address16& virtual_address, uint8_t data) override;

    private:
        enum class Mode {
            rom_banking,
            ram_banking
        };

        Mode mode{Mode::rom_banking};

        void correct_rom_bank();
        void select_memory_mode(uint8_t data);
    };
}

#endif //GAMEBOY_MBC1_H
