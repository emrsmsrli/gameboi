#ifndef GAMEBOY_MBC2_H
#define GAMEBOY_MBC2_H

#include <memory/controller/MBC.h>

namespace gameboy::memory::controller {
    class MBC2 : public MBC {
    public:
        MBC2(const std::vector<uint8_t>& rom, const CartridgeInfo& rom_header);

        void write(const Address16& virtual_address, uint8_t data) override;

    protected:
        void select_rom_bank(uint8_t data) override;
        void select_ram_bank(uint8_t data) override {};

    private:
        void control(const Address16& virtual_address, uint8_t data) override;
    };
}

#endif //GAMEBOY_MBC2_H
