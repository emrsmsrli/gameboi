#include <algorithm>
#include <numeric>

#include <cartridge.h>
#include <memory/address.h>
#include <memory/address_range.h>
#include <memory/memory_constants.h>
#include <util/observer.h>
#include <util/overloaded.h>
#include <util/data_loader.h>
#include <util/log.h>

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

template<typename T = uint8_t, typename AddrType>
static T read(const std::vector<uint8_t>& rom_data, const AddrType& addr)
{
    return static_cast<T>(rom_data[addr.value()]);
}

}

gameboy::cartridge::cartridge(std::string_view rom_path)
    : rom_(data_loader::load(rom_path))
{
    constexpr auto cgb_support_addr = make_address(0x0143u);
    constexpr auto rom_cartridge_type_addr = make_address(0x0147u);
    constexpr auto rom_ram_size_addr = make_address(0x0149u);
    constexpr auto rom_header_checksum_addr = make_address(0x014Du);

    constexpr address_range rom_header_range(0x0134u, 0x014Cu);
    constexpr address_range rom_title_range(0x0134u, 0x0142u);

    const auto checksum = std::accumulate(
        begin(rom_header_range),
        end(rom_header_range),
        static_cast<uint8_t>(0u),
        [&](const int8_t acc, const uint16_t addr) {
            return acc - rom_[addr] - 1;
        });

    if(checksum != read(rom_, rom_header_checksum_addr)) {
        log::error("rom checksum is not correct");
    }

    std::copy(
        begin(rom_) + rom_title_range.low(),
        begin(rom_) + rom_title_range.high(),
        std::back_inserter(name_));

    cgb_enabled_ = read(rom_, cgb_support_addr) != 0x00u;

    const auto xram_banks = read(rom_, rom_ram_size_addr) == 0x03u
        ? 4u
        : 1u;
    ram_.reserve(xram_banks * 8_kb);
    std::fill(begin(ram_), end(ram_), 0u);

    switch(read<mbc_type>(rom_, rom_cartridge_type_addr)) {
        case mbc_type::rom_only:
        case mbc_type::rom_ram:
        case mbc_type::rom_ram_battery: {
            // we're already here.
            break;
        }
        case mbc_type::mbc_1:
        case mbc_type::mbc_1_ram:
        case mbc_type::mbc_1_ram_battery: {
            // mbc_ = mbc1{make_observer(this)};
            break;
        }
        case mbc_type::mbc_2:
        case mbc_type::mbc_2_battery: {
            // mbc_ = mbc2{make_observer(this)};
            break;
        }
        case mbc_type::mbc_3_timer_battery:
        case mbc_type::mbc_3_timer_ram_battery:
        case mbc_type::mbc_3:
        case mbc_type::mbc_3_ram:
        case mbc_type::mbc_3_ram_battery: {
            // mbc_ = mbc3{make_observer(this)};
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
}

uint8_t gameboy::cartridge::read_rom(const address16& address) const
{
    // fixme mbc
    return rom_[address.value()];
}
void gameboy::cartridge::write_rom(const gameboy::address16& address, uint8_t data)
{
    // fixme mbc
    rom_[address.value()] = data;
}
uint8_t gameboy::cartridge::read_ram(const gameboy::address16& address) const
{
    // fixme mbc
    return ram_[address.value()];
}
void gameboy::cartridge::write_ram(const gameboy::address16& address, uint8_t data)
{
    // fixme mbc
    ram_[address.value()] = data;
}
