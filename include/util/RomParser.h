#ifndef GAMEBOY_ROMPARSER_H
#define GAMEBOY_ROMPARSER_H

#include <vector>
#include <CartridgeInfo.h>

namespace gameboy::util::rom_parser {
    CartridgeInfo parse(const std::vector<uint8_t>& rom_data);
}

#endif //GAMEBOY_ROMPARSER_H
