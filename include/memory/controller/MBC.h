//
// Created by Emre Şimşirli on 25.08.2019.
//

#ifndef GAMEBOY_MBC_H
#define GAMEBOY_MBC_H

#include <vector>
#include <cstdint>
#include "memory/Address.h"

namespace gameboy {
    class CartridgeInfo;
}

namespace gameboy::memory::controller {
    class MBC {
    public:
        explicit MBC(const std::vector<uint8_t>& rom, const CartridgeInfo& rom_header);
        virtual ~MBC() = default;

        [[nodiscard]] virtual uint8_t read(const Address16& virtual_address) const;
        virtual void write(const Address16& virtual_address, uint8_t data);

    protected:
        std::vector<uint8_t> memory;

        uint32_t n_rom_banks = 0;
        uint32_t n_video_ram_banks = 0;
        uint32_t n_external_ram_banks = 0;
        uint32_t n_working_ram_banks = 0;

        uint32_t selected_rom_bank = 0;
        uint32_t selected_ram_bank = 0;

        bool is_external_ram_enabled = false;

        [[nodiscard]] Address16 to_physical_address(const Address16& virtual_address) const;
    };

    constexpr size_t operator""_kb(unsigned long long bytes) {
        return bytes * 1024;
    }
}

#endif //GAMEBOY_MBC_H
