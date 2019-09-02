#ifndef GAMEBOY_MBC2_H
#define GAMEBOY_MBC2_H

#include <memory/controller/MBC.h>

/**
 * this memory controller works much like the MBC1    controller with the following exceptions:
 * MBC2 will work with ROM sizes up to 2Mbit.
 * Writing a value (XXXXBBBB - X = Don't cares, B =    bank select bits) into 2000-3FFF area will select
 * an appropriate ROM bank at 4000-7FFF.     RAM switching is not provided.
 * Unlike the MBC1    which uses external RAM, MBC2 has 512 x 4 bits of RAM which is in the controller itself. It still
 * requires an external battery to save data during    power-off though.     The least significant bit of the upper
 * address    byte must be zero to enable/disable cart RAM. For    example the following addresses can be used to
 * enable/disable cart RAM:    0000-00FF, 0200-02FF, 0400-04FF, ..., 1E00-1EFF.    enable/disable is 0000-00FF.
 * The suggested address    range to use for MBC2 ram     The least significant bit of the upper address
 * byte must be one to select a ROM bank. For example    the following addresses can be used to select a ROM
 * bank: 2100-21FF, 2300-23FF, 2500-25FF, ..., 3F00-    3FFF. The suggested address range to use for MBC2
 * rom bank selection is 2100-21FF.
 */
namespace gameboy::memory::controller {
    class MBC2 : public MBC {
    public:
        MBC2(const std::vector<uint8_t>& rom, const CartridgeInfo& rom_header);
    private:
        void control(const Address16& virtual_address, uint8_t data) override;
    };
}

#endif //GAMEBOY_MBC2_H
