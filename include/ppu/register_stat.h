#ifndef GAMEBOY_STAT_REGISTER_H
#define GAMEBOY_STAT_REGISTER_H

#include <cpu/register8.h>

namespace gameboy {

enum status_flag : uint8_t {
    coincidence_interrupt = 6u,
    reading_oam_interrupt = 5u,
    vblank_interrupt = 4u,
    hblank_interrupt = 3u,
    coincidence_flag = 2u,  // (0:LYC<>LY, 1:LYC=LY)
};

struct register_stat {
    register8 reg;
};

} // namespace gameboy

#endif //GAMEBOY_STAT_REGISTER_H
