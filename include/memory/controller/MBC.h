//
// Created by Emre Şimşirli on 25.08.2019.
//

#ifndef GAMEBOY_MBC_H
#define GAMEBOY_MBC_H

#include <vector>
#include <cstdint>
#include "memory/Address.h"

namespace gameboy {
    struct CartridgeInfo;
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

        uint32_t rom_bank = 0;
        uint32_t ram_bank = 0;

        bool is_external_ram_enabled = false;

        bool is_cgb = false;

        [[nodiscard]] PhysicalAddress to_physical_address(const Address16& virtual_address) const;

    private:
        [[nodiscard]] virtual uint32_t get_rom_bank() const = 0;
        [[nodiscard]] virtual uint32_t get_ram_bank() const = 0;
        [[nodiscard]] uint32_t get_video_ram_bank() const;
        [[nodiscard]] uint32_t get_work_ram_bank() const;

        virtual void control(const Address16& virtual_address, uint8_t data) = 0;
    };

    constexpr size_t operator""_kb(unsigned long long bytes) {
        return bytes * 1024u;
    }
}

#endif //GAMEBOY_MBC_H
