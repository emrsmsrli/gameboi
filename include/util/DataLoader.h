#ifndef GAMEBOY_DATALOADER_H
#define GAMEBOY_DATALOADER_H

#include <vector>
#include <cstdint>
#include <string_view>

namespace gameboy::util::data_loader {
    std::vector<uint8_t> load(std::string_view path);
}

#endif //GAMEBOY_DATALOADER_H
