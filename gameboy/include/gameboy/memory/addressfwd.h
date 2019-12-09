#ifndef GAMEBOY_ADDRESSFWD_H
#define GAMEBOY_ADDRESSFWD_H

#include <cstdint>
#include <cstddef>

namespace gameboy {

template<typename T>
class address;

/**
 * Represents an 8-bit memory address in the memory
 */
using address8 = address<uint8_t>;

/**
 * Represents a 16-bit memory address in the memory
 */
using address16 = address<uint16_t>;

/**
 * Represents a physical address in the memory (at least 32 bits)
 */
using physical_address = address<size_t>;

} // namespace gameboy

#endif //GAMEBOY_ADDRESSFWD_H
