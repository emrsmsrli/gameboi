#ifndef GAMEBOY_MMU_H
#define GAMEBOY_MMU_H

#include <cstdint>
#include <vector>

namespace gameboy::memory {
    class Address16;

    class MMU {
    public:
        void write(const Address16& address, uint8_t data);
        [[nodiscard]] uint8_t read(const Address16& address) const;

    private:
        std::vector<uint8_t> memory;
    };
}

#endif //GAMEBOY_MMU_H
