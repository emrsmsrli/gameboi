#ifndef GAMEBOY_FILEUTIL_H
#define GAMEBOY_FILEUTIL_H

#include <vector>
#include <cstdint>
#include <filesystem>

namespace filesystem = std::filesystem;

namespace gameboy {

std::vector<uint8_t> load_file(const filesystem::path& path);
void write_file(const filesystem::path&  path, const std::vector<uint8_t>& data);

} // namespace gameboy

#endif //GAMEBOY_FILEUTIL_H