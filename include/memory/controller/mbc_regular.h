#ifndef GAMEBOY_MBC_REGULAR_H
#define GAMEBOY_MBC_REGULAR_H

#include <memory/controller/mbc.h>
#include <memory/address.h>

/**
 * Rom, ram only
 */
namespace gameboy {

struct mbc_regular : public mbc {
    [[nodiscard]] uint8_t read_ram(const std::vector<uint8_t>& ram, const physical_address& address) const
    {
        return ram[address.value()];
    }

    void write_ram(std::vector<uint8_t>& ram, const physical_address& address, const uint8_t data) const
    {
        ram[address.value()] = data;
    }
};

} // namespace gameboy

#endif //GAMEBOY_MBC_REGULAR_H
