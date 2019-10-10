#ifndef GAMEBOY_MBC2_H
#define GAMEBOY_MBC2_H

#include <memory/controller/mbc.h>

namespace gameboy {

class mbc2 : public mbc {
public:
    mbc2(const std::vector<uint8_t>& rom, const cartridge& rom_header);

    void write(const address16& virtual_address, uint8_t data) override;

protected:
    void select_rom_bank(uint8_t data) override;
    void select_ram_bank(uint8_t data) override { };

private:
    void control(const address16& virtual_address, uint8_t data) override;
};

}

#endif //GAMEBOY_MBC2_H
