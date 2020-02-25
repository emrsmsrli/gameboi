#include <fstream>
#include <iterator>

#include <spdlog/spdlog.h>

#include "gameboy/util/data_loader.h"

namespace gameboy {

std::vector<uint8_t> data_loader::load(const std::string_view path)
{
    std::ifstream file(path.data(), std::ios::binary);
    file.unsetf(std::ios::skipws);

    if(!file.is_open()) {
        spdlog::critical("file could not be opened: {}", path);
    }

    std::vector<uint8_t> bytes{
        std::istream_iterator<uint8_t>{file},
        std::istream_iterator<uint8_t>{}
    };

    return bytes;
}

} // namespace gameboy
