#ifndef GAMEBOY_MBC1_H
#define GAMEBOY_MBC1_H

#include <cstdint>
#include <vector>

#include "gameboy/memory/controller/mbc.h"
#include "gameboy/memory/addressfwd.h"

namespace gameboy {

class mbc1 : public mbc {
public:
    explicit mbc1(const observer<cartridge> cartridge)
        : mbc(cartridge) {}

    void control(const address16& address, uint8_t data) noexcept;

    [[nodiscard]] uint8_t read_ram(const physical_address& address) const;
    void write_ram(const physical_address& address, uint8_t data);

    [[nodiscard]] bool rom_banking_active() const noexcept { return rom_banking_active_; }

private:
    bool rom_banking_active_ = true;
};

} // namespace gameboy

#endif //GAMEBOY_MBC1_H
