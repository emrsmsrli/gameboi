#include <fstream>
#include <iterator>
#include <util/DataLoader.h>

namespace {
    auto get_file_size(std::ifstream& rom_file)
    {
        const auto rom_size = rom_file.tellg();
        rom_file.seekg(0, std::ios::beg);
        return rom_size;
    }
}

std::vector<uint8_t> gameboy::util::data_loader::load(std::string_view path)
{
    std::ifstream file(path.data(), std::ios::binary | std::ios::ate);
    file.unsetf(std::ios::skipws);

    const auto rom_size = get_file_size(file);

    std::vector<uint8_t> bytes(rom_size);

    // read the data
    bytes.insert(bytes.begin(),
            std::istream_iterator<uint8_t>(file),
            std::istream_iterator<uint8_t>());

    return bytes;
}
