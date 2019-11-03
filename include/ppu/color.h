#ifndef GAMEBOY_COLOR_H
#define GAMEBOY_COLOR_H

#include <cstdint>

namespace gameboy {

struct color {
    uint8_t red = 0u;
    uint8_t green = 0u;
    uint8_t blue = 0u;
};

} // namespace gameboy

#endif //GAMEBOY_COLOR_H
