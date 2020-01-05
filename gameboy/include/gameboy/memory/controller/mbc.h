#ifndef GAMEBOY_MBC_H
#define GAMEBOY_MBC_H

#include <cstdint>

namespace gameboy {

class cartridge;

struct mbc {
    /** in range [0, N) */
    uint32_t rom_bank = 1u;

    /** in range [0, N] */
    uint32_t ram_bank = 0u;

    bool xram_enabled = false;

    void set_xram_enabled(const uint8_t data) noexcept { xram_enabled = (data & 0x0Fu) == 0x0Au; }
};

} // namespace gameboy

#endif //GAMEBOY_MBC_H
