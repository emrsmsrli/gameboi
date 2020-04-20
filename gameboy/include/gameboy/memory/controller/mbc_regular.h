#ifndef GAMEBOY_MBC_REGULAR_H
#define GAMEBOY_MBC_REGULAR_H

#include "gameboy/memory/address.h"
#include "gameboy/memory/controller/mbc.h"

/**
 * Rom, ram only
 */
namespace gameboy {

class mbc_regular : public mbc {
public:
    explicit mbc_regular(const observer<cartridge> cartridge)
        : mbc(cartridge) {}

    [[nodiscard]] uint8_t read_ram(const physical_address& address) const;
    void write_ram(const physical_address& address, uint8_t data) const;
};

} // namespace gameboy

#endif //GAMEBOY_MBC_REGULAR_H
