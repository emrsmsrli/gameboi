#ifndef GAMEBOY_ROM_PARSER_H
#define GAMEBOY_ROM_PARSER_H

#include <vector>
#include <cartridge.h>

namespace gameboy::util::rom_parser {
    cartridge parse(const std::vector<uint8_t>& rom_data);
}

#endif //GAMEBOY_ROM_PARSER_H
