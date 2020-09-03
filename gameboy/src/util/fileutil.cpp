#include "gameboy/util/fileutil.h"

#include <fstream>
#include <iterator>

#include <spdlog/spdlog.h>

namespace gameboy {

std::vector<uint8_t> read_file(const filesystem::path& path)
{
    std::ifstream stream{path, std::ios::binary | std::ios::in};
    stream.unsetf(std::ios::skipws);
    if(!stream.is_open()) {
        spdlog::critical("stream could not be opened: {}", path.string());
        std::terminate();
    }

    std::vector<uint8_t> bytes{
        std::istream_iterator<uint8_t>{stream},
        std::istream_iterator<uint8_t>{}
    };

    return bytes;
}

void write_file(const filesystem::path& path, const std::vector<uint8_t>& data)
{
    std::ofstream stream{path, std::ios::binary | std::ios::out};
    stream.unsetf(std::ios::skipws);
    if(!stream.is_open()) {
        spdlog::critical("stream could not be opened: {}", path.string());
        std::terminate();
    }

    stream.write(reinterpret_cast<const char*>(data.data()), data.size());
}

} // namespace gameboy
