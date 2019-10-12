#include <cstdint>
#include <numeric>
#include <array>

#include <util/rom_parser.h>
#include <util/data_loader.h>
#include <util/log.h>
#include <memory/address_range.h>
#include <memory/controller/mbc_null.h>
#include <memory/controller/mbc1.h>

namespace {

enum class mbc_type : uint8_t {
    rom_only = 0x00u,
    mbc_1 = 0x01u,
    mbc_1_ram = 0x02u,
    mbc_1_ram_battery = 0x03u,
    mbc_2 = 0x05u,
    mbc_2_battery = 0x06u,
    rom_ram = 0x08u,
    rom_ram_battery = 0x09u,
    mmm_01 = 0x0Bu,
    mmm_01_ram = 0x0Cu,
    mmm_01_ram_battery = 0x0Du,
    mbc_3_timer_battery = 0x0Fu,
    mbc_3_timer_ram_battery = 0x10u,
    mbc_3 = 0x11u,
    mbc_3_ram = 0x12u,
    mbc_3_ram_battery = 0x13u,
    mbc_4 = 0x15u,
    mbc_4_ram = 0x16u,
    mbc_4_ram_battery = 0x17u,
    mbc_5 = 0x19u,
    mbc_5_ram = 0x1Au,
    mbc_5_ram_battery = 0x1Bu,
    mbc_5_rumble = 0x1Cu,
    mbc_5_rumble_ram = 0x1Du,
    mbc_5_rumble_ram_battery = 0x1Eu,
    pocket_camera = 0xFCu,
    bandai_tama_5 = 0xFDu,
    huc_3 = 0xFEu,
    huc_1_ram_battery = 0xFFu
};

}

gameboy::cartridge gameboy::rom_parser::parse(const std::string_view rom_path)
{
    constexpr std::array<uint32_t, 11> rom_size_to_banks{2, 4, 8, 16, 32, 64, 128, 256, 72, 80, 96};

    constexpr auto cgb_support_addr = make_address(0x0143u);
    constexpr auto rom_cartridge_type_addr = make_address(0x0147u);
    constexpr auto rom_rom_size_addr = make_address(0x0148u);
    constexpr auto rom_ram_size_addr = make_address(0x0149u);
    constexpr auto rom_header_checksum_addr = make_address(0x014Du);

    constexpr address_range rom_header_range(0x0134u, 0x014Cu);
    constexpr address_range rom_title_range(0x0134u, 0x0142u);

    const auto rom_data = data_loader::load(rom_path);

    cartridge cartridge;
    std::copy(
        begin(rom_data) + rom_title_range.get_low(),
        begin(rom_data) + rom_title_range.get_high(),
        std::back_inserter(cartridge.name_));

    cartridge.cgb_enabled_ = read(rom_data, cgb_support_addr) != 0x00;
    cartridge.rom_.reserve(rom_size_to_banks[read(rom_data, rom_rom_size_addr)]);
    cartridge.ram_.reserve(rom_size_to_banks[read(rom_data, rom_ram_size_addr)]);

    switch(read<mbc_type>(rom_data, rom_cartridge_type_addr)) {
        case mbc_type::rom_only:
        case mbc_type::rom_ram:
        case mbc_type::rom_ram_battery: {
            cartridge.mbc_ = mbc_null{};
            break;
        }
        case mbc_type::mbc_1:
        case mbc_type::mbc_1_ram:
        case mbc_type::mbc_1_ram_battery: {
            cartridge.mbc_ = mbc1{};
            break;
        }
        case mbc_type::mbc_2:
        case mbc_type::mbc_2_battery: {
            cartridge.mbc_ = mbc2{};
            break;
        }
        case mbc_type::mbc_3_timer_battery:
        case mbc_type::mbc_3_timer_ram_battery:
        case mbc_type::mbc_3:
        case mbc_type::mbc_3_ram:
        case mbc_type::mbc_3_ram_battery: {
            cartridge.mbc_ = mbc3{};
            break;
        }
        case mbc_type::mmm_01:
        case mbc_type::mmm_01_ram:
        case mbc_type::mmm_01_ram_battery:
        case mbc_type::mbc_4:
        case mbc_type::mbc_4_ram:
        case mbc_type::mbc_4_ram_battery:
        case mbc_type::mbc_5:
        case mbc_type::mbc_5_ram:
        case mbc_type::mbc_5_ram_battery:
        case mbc_type::mbc_5_rumble:
        case mbc_type::mbc_5_rumble_ram:
        case mbc_type::mbc_5_rumble_ram_battery:
        case mbc_type::pocket_camera:
        case mbc_type::bandai_tama_5:
        case mbc_type::huc_3:
        case mbc_type::huc_1_ram_battery:
        default: {
            log::error("unimplemented cartridge type");
        }
    }

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

    return cartridge;
}
