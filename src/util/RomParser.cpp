
#include <cstdint>
#include <numeric>
#include "util/Log.h"
#include "util/RomParser.h"
#include "memory/AddressMap.h"
#include "memory/AddressRange.h"

gameboy::CartridgeInfo gameboy::util::rom_parser::parse(const std::vector<uint8_t>& rom_data)
{
    CartridgeInfo info;
    std::copy(
            begin(rom_data) + memory::Map::rom_title_begin,
            begin(rom_data) + memory::Map::rom_title_end,
            std::back_inserter(info.name));
    info.cgb_support = static_cast<CartridgeInfo::GameBoyColorSupport>(rom_data[memory::Map::rom_cgb_support]);
    info.sgb_support = static_cast<CartridgeInfo::SuperGameBoySupport>(rom_data[memory::Map::rom_sgb_support]);
    info.cartridge_type = static_cast<CartridgeInfo::Type>(rom_data[memory::Map::rom_cartridge_type]);
    info.rom_size = static_cast<CartridgeInfo::RomSize>(rom_data[memory::Map::rom_rom_size]);
    info.ram_size = static_cast<CartridgeInfo::RamSize>(rom_data[memory::Map::rom_ram_size]);

    constexpr memory::AddressRange range(memory::Map::rom_header_begin, memory::Map::rom_header_end);
    const auto checksum = std::accumulate(
            begin(range),
            end(range),
            static_cast<uint8_t>(0),
            [&](int8_t acc, uint16_t addr) {
                return acc - rom_data[addr] - 1;
            });

    if(checksum != rom_data[memory::Map::rom_header_checksum]) {
        log::error("rom checksum is not correct");
    }

    return info;
}
