#ifndef GAMEBOY_FILEUTIL_H
#define GAMEBOY_FILEUTIL_H

#include <cstdint>
#include <filesystem>
#include <vector>

namespace filesystem = std::filesystem;

namespace gameboy {

std::vector<uint8_t> read_file(const filesystem::path& path);
void write_file(const filesystem::path&  path, const std::vector<uint8_t>& data);

} // namespace gameboy

#endif //GAMEBOY_FILEUTIL_H
