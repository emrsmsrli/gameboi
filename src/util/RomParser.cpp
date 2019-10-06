#include <cstdint>
#include <numeric>
#include <util/Log.h>
#include <util/RomParser.h>
#include <memory/AddressRange.h>

namespace {
    template<typename T = uint8_t, typename AddrType>
    T read(const std::vector<uint8_t>& rom_data, const AddrType& addr)
    {
        return static_cast<T>(rom_data[addr.get_value()]);
    }
}

gameboy::CartridgeInfo gameboy::util::rom_parser::parse(const std::vector<uint8_t>& rom_data)
{
    static constexpr auto cgb_support_addr = memory::make_address(0x0143u);
    static constexpr auto sgb_support_addr = memory::make_address(0x0146u);
    static constexpr auto rom_cartridge_type_addr = memory::make_address(0x0147u);
    static constexpr auto rom_rom_size_addr = memory::make_address(0x0148u);
    static constexpr auto rom_ram_size_addr = memory::make_address(0x0149u);
    static constexpr auto rom_header_checksum_addr = memory::make_address(0x014Du);

    static constexpr memory::AddressRange rom_header_range(0x0134u, 0x014Cu);
    static constexpr memory::AddressRange rom_title_range(0x0134u, 0x0142u);

    CartridgeInfo info;
    std::copy(
            begin(rom_data) + rom_title_range.get_low(),
            begin(rom_data) + rom_title_range.get_high(),
            std::back_inserter(info.name));
    info.cgb_support = read<CartridgeInfo::GameBoyColorSupport>(rom_data, cgb_support_addr);
    info.sgb_support = read<CartridgeInfo::SuperGameBoySupport>(rom_data, sgb_support_addr);
    info.cartridge_type = read<CartridgeInfo::Type>(rom_data, rom_cartridge_type_addr);
    info.rom_size = read<CartridgeInfo::RomSize>(rom_data, rom_rom_size_addr);
    info.ram_size = read<CartridgeInfo::RamSize>(rom_data, rom_ram_size_addr);

    const auto checksum = std::accumulate(
            begin(rom_header_range),
            end(rom_header_range),
            static_cast<uint8_t>(0u),
            [&](const int8_t acc, const uint16_t addr) {
                return acc - rom_data[addr] - 1;
            });

    if(checksum != read(rom_data, rom_header_checksum_addr)) {
        log::error("rom checksum is not correct");
    }

    return info;
}
