#ifndef GAMEBOY_ROMLOADER_H
#define GAMEBOY_ROMLOADER_H

#include <vector>
#include <cstdint>
#include <string_view>

namespace gameboy::util::rom_loader {
    std::vector<uint8_t> load(std::string_view path);
}

#endif //GAMEBOY_ROMLOADER_H
