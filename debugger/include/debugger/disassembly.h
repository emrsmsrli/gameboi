#ifndef GAMEBOY_DISASSEMBLY_H
#define GAMEBOY_DISASSEMBLY_H

#include <string>

#include "gameboy/cpu/instruction_info.h"
#include "gameboy/memory/address.h"

namespace gameboy::instruction {

struct disassembly {
    uint32_t bank = 0u;
    address16 address{0u};
    instruction::info info;
    std::string representation;

    disassembly(const uint32_t bank, const address16 address, const instruction::info& info, std::string representation)
        : bank{bank}, address{address}, info(info), representation{std::move(representation)} {}
};

} // namespace gameboy::instruction

#endif //GAMEBOY_DISASSEMBLY_H
