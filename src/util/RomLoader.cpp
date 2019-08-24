
#include "util/RomLoader.h"
#include <fstream>
#include <iterator>

namespace {
    auto get_rom_size(std::ifstream& rom_file) {
        const auto rom_size = rom_file.tellg();
        rom_file.seekg(0, std::ios::beg);
        return rom_size;
    }
}

std::vector<uint8_t> gameboy::util::rom_loader::load(std::string_view path)
{
    std::ifstream rom_file(path.data(), std::ios::binary | std::ios::ate);
    rom_file.unsetf(std::ios::skipws);

    const auto rom_size = get_rom_size(rom_file);

    std::vector<uint8_t> bytes(rom_size);

    // read the data:
    bytes.insert(bytes.begin(),
            std::istream_iterator<uint8_t>(rom_file),
            std::istream_iterator<uint8_t>());

    return bytes;
}
