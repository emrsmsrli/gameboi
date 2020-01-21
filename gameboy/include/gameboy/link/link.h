#ifndef GAMEBOY_LINK_H
#define GAMEBOY_LINK_H

#include <cstdint>

namespace gameboy {

class link {
public:
    void tick(uint8_t cycles) noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_LINK_H