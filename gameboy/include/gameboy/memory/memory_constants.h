#ifndef GAMEBOY_MEMORY_CONSTANTS_H
#define GAMEBOY_MEMORY_CONSTANTS_H

#include "gameboy/memory/address_range.h"

namespace gameboy {

static constexpr address_range rom_range{0x0000u, 0x7FFFu};
static constexpr address_range vram_range{0x8000u, 0x9FFFu};
static constexpr address_range xram_range{0xA000u, 0xBFFFu};
static constexpr address_range wram_range{0xC000u, 0xDFFFu};
static constexpr address_range echo_range{0xE000u, 0xFDFFu};
static constexpr address_range oam_range{0xFE00u, 0xFE9Fu};
static constexpr address_range wave_pattern_range{0xFF30u, 0xFF3Fu};
static constexpr address_range hram_range{0xFF80u, 0xFFFEu};

} // namespace gameboy

#endif //GAMEBOY_MEMORY_CONSTANTS_H
