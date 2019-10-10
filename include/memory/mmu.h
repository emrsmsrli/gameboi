#ifndef GAMEBOY_MMU_H
#define GAMEBOY_MMU_H

#include <cstdint>
#include <vector>
#include <memory>
#include <memory/controller/mbc.h>
#include <memory/addressfwd.h>

namespace gameboy {
    class mmu {
    public:
        void initialize() const;

        void write(const address16& address, uint8_t data) const;
        [[nodiscard]] uint8_t read(const address16& address) const;

        void load_rom(const std::vector<uint8_t>& rom_data);

        void load_external_memory(const std::vector<uint8_t>& save_data);
        [[nodiscard]] std::vector<uint8_t> copy_external_memory() const;

    private:
        std::unique_ptr<mbc> mbc_;
    };
}

#endif //GAMEBOY_MMU_H
