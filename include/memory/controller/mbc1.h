#ifndef GAMEBOY_MBC1_H
#define GAMEBOY_MBC1_H

#include <cstdint>

#include <memory/controller/mbc.h>

namespace gameboy {

class mbc1 : public mbc {
public:
    mbc1(const std::vector<uint8_t>& rom, const cartridge& rom_header);

protected:
    void select_rom_bank(uint8_t data) override;
    void select_ram_bank(uint8_t data) override;

private:
    /** if false, ram banking is active */
    bool is_rom_banking_active_ = true;

    void select_memory_mode(uint8_t data);

    [[nodiscard]] uint32_t get_rom_bank() const override;
    [[nodiscard]] uint32_t get_ram_bank() const override;

    void control(const address16& virtual_address, uint8_t data) override;
};

}

#endif //GAMEBOY_MBC1_H
