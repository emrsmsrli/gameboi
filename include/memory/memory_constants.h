#ifndef GAMEBOY_MEMORY_CONSTANTS_H
#define GAMEBOY_MEMORY_CONSTANTS_H

#include <memory/address_range.h>

namespace gameboy {

static constexpr gameboy::address_range rom_range{0x0000u, 0x7FFFu};
static constexpr gameboy::address_range vram_range{0x8000u, 0x9FFFu};
static constexpr gameboy::address_range xram_range{0xA000u, 0xBFFFu};
static constexpr gameboy::address_range wram_range{0xC000u, 0xDFFFu};

}

#endif //GAMEBOY_MEMORY_CONSTANTS_H
