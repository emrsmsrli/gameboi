//
// Created by Emre Şimşirli on 1.09.2019.
//

#ifndef GAMEBOY_ADDRESSFWD_H
#define GAMEBOY_ADDRESSFWD_H

#include <cstdint>
#include <cstddef>

namespace gameboy::memory {
    template<typename T>
    class Address;

    /**
     * Represents an 8-bit memory address in the memory
     */
    using Address8 = Address<uint8_t>;

    /**
     * Represents a 16-bit memory address in the memory
     */
    using Address16 = Address<uint16_t>;

    /**
     * Represents a physical address in the memory (at least 32 bits)
     */
    using PhysicalAddress = Address<size_t>;
}

#endif //GAMEBOY_ADDRESSFWD_H
