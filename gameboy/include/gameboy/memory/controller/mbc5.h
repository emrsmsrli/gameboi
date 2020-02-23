#ifndef GAMEBOY_MBC5_H
#define GAMEBOY_MBC5_H

#include <cstdint>
#include <vector>

#include "gameboy/memory/controller/mbc.h"
#include "gameboy/memory/addressfwd.h"

namespace gameboy {

struct mbc5 : mbc {
    bool rom_banking_active = true;

    void control(const address16& address, uint8_t data) noexcept;

    [[nodiscard]] uint8_t read_ram(const std::vector<uint8_t>& ram, const physical_address& address) const;
    void write_ram(std::vector<uint8_t>& ram, const physical_address& address, uint8_t data) const;
};

} // namespace gameboy

#endif //GAMEBOY_MBC5_H
