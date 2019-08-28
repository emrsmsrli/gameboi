#ifndef GAMEBOY_MMU_H
#define GAMEBOY_MMU_H

#include <cstdint>
#include <vector>
#include <memory>
#include "memory/controller/MBC.h"

namespace gameboy::memory {
    class Address16;

    class MMU {
    public:
        void write(const Address16& address, uint8_t data);
        [[nodiscard]] uint8_t read(const Address16& address) const;

    private:
        std::unique_ptr<controller::MBC> mbc;
    };
}

#endif //GAMEBOY_MMU_H
