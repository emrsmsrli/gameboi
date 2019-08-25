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
        constexpr Address8() = default;
        constexpr explicit Address8(uint8_t default_value) : value(default_value) {}

        [[nodiscard]] constexpr uint8_t get_value() const { return value; }

    private:
        uint8_t value = 0x00;
    };

    /**
     * Represents a 16-bit memory address in the memory
     */
    class Address16 {
    public:
        constexpr Address16() = default;
        constexpr explicit Address16(uint16_t default_value) : value(default_value) {}

        [[nodiscard]] constexpr uint16_t get_value() const { return value; }

    private:
        uint8_t value = 0x0000;
    };

    enum class MemoryMapLocation : uint16_t {
        rom_bank_0_start = 0x0000,
        rom_bank_0_end = 0x3FFF,
        rom_bank_switchable_start = 0x4000,
        rom_bank_switchable_end = 0x7FFF,
        vram_start = 0x8000,
        vram_end = 0x9FFF,
        ram_bank_switchable_start = 0xA000,
        ram_bank_switchable_end = 0xBFFF,
        ram_internal_start = 0xC000,
        ram_internal_end = 0xDFFF,
        ram_internal_echo_start = 0xE000,
        ram_internal_echo_end = 0xFDFF,
        object_attribute_memory_start = 0xFE00,
        object_attribute_memory_end = 0xFE9F,
        io_start = 0xFF00,
        io_end = 0xFF4B,
        ram_high_start = 0xFF80,
        ram_high_end = 0xFFFE,
        register_interrupt_enable = 0xFFFF
    };

    /**
     * Makes an address object
     * @param location two byte memory location
     * @return An address object
     */
    Address16 make_address(MemoryMapLocation location);

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
