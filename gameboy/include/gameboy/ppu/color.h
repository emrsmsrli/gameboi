#ifndef GAMEBOY_COLOR_H
#define GAMEBOY_COLOR_H

#include <cstdint>

namespace gameboy {

struct color {
    uint8_t red = 0u;
    uint8_t green = 0u;
    uint8_t blue = 0u;

    constexpr color() = default;
    constexpr explicit color(const uint8_t intensity)
	    : color{intensity, intensity, intensity} {}
    constexpr color(const uint8_t r, const uint8_t g, const uint8_t b)
	    : red{r}, green{g}, blue{b} {}
};

} // namespace gameboy

#endif //GAMEBOY_COLOR_H
