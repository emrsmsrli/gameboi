#ifndef GAMEBOY_MBC5_H
#define GAMEBOY_MBC5_H

#include <cstdint>
#include <vector>

#include "gameboy/memory/addressfwd.h"
#include "gameboy/memory/controller/mbc.h"

namespace gameboy {

class mbc5 : public mbc {
public:
    explicit mbc5(const observer<cartridge> cartridge)
        : mbc(cartridge) {}

    void control(const address16& address, uint8_t data) noexcept;

    [[nodiscard]] uint8_t read_ram(const physical_address& address) const;
    void write_ram(const physical_address& address, uint8_t data);
};

} // namespace gameboy

#endif //GAMEBOY_MBC5_H
