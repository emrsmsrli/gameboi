#ifndef GAMEBOY_MBC_H
#define GAMEBOY_MBC_H

#include <cstdint>

#include "gameboy/util/observer.h"

namespace gameboy {

class cartridge;

class mbc {
public:
    explicit mbc(const observer<cartridge> cartridge) : cartridge_{cartridge} {}

    void set_ram_enabled(const uint8_t data) noexcept { ram_enabled_ = (data & 0x0Fu) == 0x0Au; }
    [[nodiscard]] bool is_ram_enabled() const noexcept { return ram_enabled_; }
    [[nodiscard]] uint32_t rom_bank() const noexcept { return rom_bank_; }
    [[nodiscard]] uint32_t ram_bank() const noexcept { return ram_bank_; }

protected:
    observer<cartridge> cartridge_;

    uint32_t rom_bank_ = 1u;
    uint32_t ram_bank_ = 0u;
    bool ram_enabled_ = false;
};

} // namespace gameboy

#endif //GAMEBOY_MBC_H
