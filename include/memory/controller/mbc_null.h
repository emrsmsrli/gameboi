#ifndef GAMEBOY_MBC_NULL_H
#define GAMEBOY_MBC_NULL_H

#include <memory/controller/mbc.h>

/**
 * Rom only
 */
namespace gameboy {

class mbc_null : public mbc {
public:
    mbc_null(const std::vector<uint8_t>& rom, const cartridge& rom_header)
        :mbc(rom, rom_header) { }

protected:
    void select_rom_bank(uint8_t data) override { }
    void select_ram_bank(uint8_t data) override { }

private:
    void control(const address16&, uint8_t) override { }
};

}

#endif //GAMEBOY_MBC_NULL_H
