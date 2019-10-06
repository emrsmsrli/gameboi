#ifndef GAMEBOY_MMU_H
#define GAMEBOY_MMU_H

#include <cstdint>
#include <vector>
#include <memory>
#include <memory/controller/MBC.h>
#include <memory/AddressFwd.h>

namespace gameboy::memory {
    class MMU {
    public:
        void initialize() const;

        void write(const Address16& address, uint8_t data) const;
        [[nodiscard]] uint8_t read(const Address16& address) const;

        void load_rom(const std::vector<uint8_t>& rom_data);

        void load_external_memory(const std::vector<uint8_t>& save_data);
        [[nodiscard]] std::vector<uint8_t> copy_external_memory() const;

    private:
        std::unique_ptr<controller::MBC> mbc;
    };
}

#endif //GAMEBOY_MMU_H
