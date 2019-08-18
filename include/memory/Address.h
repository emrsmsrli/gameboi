#ifndef GAMEBOY_ADDRESS_H
#define GAMEBOY_ADDRESS_H

#include <cstdint>

namespace gameboy::cpu {
    class Register8;
    class Register16;
}

namespace gameboy::memory {
    /**
     * Represents an 8-bit memory address in the memory
     */
    class Address8 {
    public:
        Address8() = default;
        explicit Address8(uint8_t default_value) : value(default_value) {}

        [[nodiscard]] uint8_t get_value() const { return value; }

    private:
        uint8_t value = 0x00;
    };

    /**
     * Represents a 16-bit memory address in the memory
     */
    class Address16 {
    public:
        Address16() = default;
        explicit Address16(uint16_t default_value) : value(default_value) {}

        [[nodiscard]] uint16_t get_value() const { return value; }

    private:
        uint8_t value = 0x0000;
    };

    /**
     * Makes an address object
     * @param address one byte address value
     * @return An address object
     */
    Address8 make_address(uint8_t address);

    /**
     * Makes an address object
     * @param address two byte address value
     * @return An address object
     */
    Address16 make_address(uint16_t address);

    /**
     * Makes an address object using a 16-bit register
     * @param reg a 16-bit register
     * @return An address object
     */
    Address16 make_address(const cpu::Register16& reg);
}

#endif //GAMEBOY_ADDRESS_H
