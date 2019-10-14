#ifndef GAMEBOY_MBC_REGULAR_H
#define GAMEBOY_MBC_REGULAR_H

#include <memory/controller/mbc.h>

/**
 * Rom, ram only
 */
namespace gameboy {

struct mbc_regular : public mbc {
    [[nodiscard]] uint8_t read_ram(const std::vector<uint8_t>& ram, size_t address) const noexcept
    {
        return ram[address];
    }

    void write_ram(std::vector<uint8_t>& ram, const size_t address, uint8_t data) const noexcept
    {
        ram[address] = data;
    }
};

}

#endif //GAMEBOY_MBC_REGULAR_H
