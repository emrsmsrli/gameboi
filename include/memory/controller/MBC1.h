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
        /** if false, ram banking is active */
        bool is_rom_banking_active = true;

        void select_rom_bank(uint8_t data);
        void select_ram_bank(uint8_t data);
        void select_memory_mode(uint8_t data);

        [[nodiscard]] uint32_t get_rom_bank() const override;
        [[nodiscard]] uint32_t get_ram_bank() const override;
    };
}

#endif //GAMEBOY_MBC1_H
