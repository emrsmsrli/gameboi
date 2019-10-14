#ifndef GAMEBOY_MBC2_H
#define GAMEBOY_MBC2_H

#include <cstdint>

#include <memory/controller/mbc.h>

namespace gameboy {

struct mbc2 : public mbc {
    void control(const address16& address, uint8_t data) noexcept;

    [[nodiscard]] uint8_t read_ram(const std::vector<uint8_t>& ram, size_t address) const noexcept;
    void write_ram(std::vector<uint8_t>& ram, size_t address, uint8_t data) const noexcept;
};

}

#endif //GAMEBOY_MBC2_H
