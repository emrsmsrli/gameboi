#ifndef GAMEBOY_ADDRESS_H
#define GAMEBOY_ADDRESS_H

#include <cstdint>

namespace gameboy::memory {
    class Address8 {
    public:
        Address8() = default;
        explicit Address8(uint8_t default_value) : value(default_value) {}

        [[nodiscard]] uint8_t get_value() const { return value; }

    private:
        uint8_t value = 0x00;
    };

    class Address16 {
    public:
        Address16() = default;
        explicit Address16(uint16_t default_value) : value(default_value) {}

        [[nodiscard]] uint16_t get_value() const { return value; }

    private:
        uint8_t value = 0x0000;
    };
}

#endif //GAMEBOY_ADDRESS_H
