#ifndef GAMEBOY_ROM_PARSER_H
#define GAMEBOY_ROM_PARSER_H

#include <string_view>

#include <cartridge.h>

namespace gameboy {

class rom_parser {
public:
    static cartridge parse(std::string_view rom_path);

private:
    template<typename T = uint8_t, typename AddrType>
    static T read(const std::vector<uint8_t>& rom_data, const AddrType& addr)
    {
        return static_cast<T>(rom_data[addr.get_value()]);
    }
};

}

#endif //GAMEBOY_ROM_PARSER_H
