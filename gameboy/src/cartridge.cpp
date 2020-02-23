#include <algorithm>
#include <numeric>

#include <magic_enum.hpp>

#include "gameboy/cartridge.h"
#include "gameboy/memory/address.h"
#include "gameboy/memory/address_range.h"
#include "gameboy/memory/memory_constants.h"
#include "gameboy/util/overloaded.h"
#include "gameboy/util/data_loader.h"
#include "gameboy/util/log.h"

namespace gameboy {

enum class cgb_type : uint8_t {
    only_gb = 0x00u,
    supports_cgb = 0x80u,
    only_cgb = 0xC0u
};

enum class rom_type : uint8_t {
    kb_32 = 0x00u,
    kb_64 = 0x01u,
    kb_128 = 0x02u,
    kb_256 = 0x03u,
    kb_512 = 0x04u,
    mb_1 = 0x05u,
    mb_2 = 0x06u,
    mb_5 = 0x07u,
    mb_1_1 = 0x52u,
    mb_1_2 = 0x53u,
    mb_1_5 = 0x54u
};

enum class ram_type : uint8_t {
    none = 0x00u,
    kb_2 = 0x01u,
    kb_8 = 0x02u,
    kb_32 = 0x03u
};

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
[[nodiscard]] T read(const std::vector<uint8_t>& rom_data, const AddrType& addr)
{
    return static_cast<T>(rom_data[addr.value()]);
}

cartridge::cartridge(const std::string_view rom_path)
    : rom_{data_loader::load(rom_path)}
{
    constexpr auto cgb_support_addr = make_address(0x0143u);
    constexpr auto mbc_type_addr = make_address(0x0147u);
    constexpr auto rom_size_addr = make_address(0x0148u);
    constexpr auto xram_size_addr = make_address(0x0149u);
    constexpr auto header_checksum_addr = make_address(0x014Du);

    constexpr address_range rom_header_range(0x0134u, 0x014Cu);
    constexpr address_range rom_title_range(0x0134u, 0x0142u);

    const auto checksum = std::accumulate(
        begin(rom_header_range),
        end(rom_header_range),
        static_cast<uint8_t>(0u),
        [&](const uint8_t acc, const uint16_t addr) {
            return acc - rom_[addr] - 1;
        });

    if(const auto expected = read(rom_, header_checksum_addr); checksum != expected) {
        log::error("rom checksum is not correct. expected: {}, calculated: {}", expected, checksum);
    }

    std::copy(
        begin(rom_) + *begin(rom_title_range),
        begin(rom_) + *end(rom_title_range),
        std::back_inserter(name_));

    const auto cgb = read<cgb_type>(rom_, cgb_support_addr);
    cgb_enabled_ = cgb != cgb_type::only_gb;
    cgb_type_ = magic_enum::enum_name(cgb);

    rom_type_ = magic_enum::enum_name(read<rom_type>(rom_, rom_size_addr));

    const auto xram_type = read<ram_type>(rom_, xram_size_addr);
    const auto xram_banks = xram_type == ram_type::kb_32
                            ? 4u
                            : 1u;
    ram_.resize(xram_banks * 8_kb);
    std::fill(begin(ram_), end(ram_), 0u);
    ram_type_ = magic_enum::enum_name(xram_type);

    const auto mbc = read<mbc_type>(rom_, mbc_type_addr);
    mbc_type_ = magic_enum::enum_name(mbc);
    switch(mbc) {
        case mbc_type::rom_only:
        case mbc_type::rom_ram:
        case mbc_type::rom_ram_battery: {
            // we're already here.
            break;
        }
        case mbc_type::mbc_1:
        case mbc_type::mbc_1_ram:
        case mbc_type::mbc_1_ram_battery: {
            mbc_ = mbc1{};
            break;
        }
        case mbc_type::mbc_2:
        case mbc_type::mbc_2_battery: {
            mbc_ = mbc2{};
            break;
        }
        case mbc_type::mbc_3_timer_battery:
        case mbc_type::mbc_3_timer_ram_battery:
        case mbc_type::mbc_3:
        case mbc_type::mbc_3_ram:
        case mbc_type::mbc_3_ram_battery: {
            mbc_ = mbc3{};
            break;
        }
        case mbc_type::mbc_5:
        case mbc_type::mbc_5_ram:
        case mbc_type::mbc_5_ram_battery:
        case mbc_type::mbc_5_rumble:
        case mbc_type::mbc_5_rumble_ram:
        case mbc_type::mbc_5_rumble_ram_battery: {
            // todo mbc_ = mbc5{};
            break;
        }
        default: {
            log::error("unimplemented cartridge type {}", magic_enum::underlying_type_t<mbc_type>{});
        }
    }

    switch(mbc) {
        case mbc_type::mbc_1_ram_battery:
        case mbc_type::mbc_2_battery:
        case mbc_type::rom_ram_battery:
        case mbc_type::mmm_01_ram_battery:
        case mbc_type::mbc_3_timer_battery:
        case mbc_type::mbc_3_timer_ram_battery:
        case mbc_type::mbc_3_ram_battery:
        case mbc_type::mbc_5_ram_battery:
        case mbc_type::mbc_5_rumble_ram_battery:
        case mbc_type::huc_1_ram_battery:
            has_battery_ = true;
            break;
        default:
            break;
    }
}

uint8_t cartridge::read_rom(const address16& address) const
{
    const auto physical_addr = [&]() -> size_t {
        static constexpr address_range first_rom_bank_range{0x3FFFu};
        if(first_rom_bank_range.has(address)) {
            return address.value();
        }

        return address.value() + 16_kb * (rom_bank() - 1u);
    }();

    return rom_[physical_addr];
}

void cartridge::write_rom(const address16& address, uint8_t data)
{
    std::visit(overloaded{
        [](mbc_regular&) {},
        [&](auto&& mbc) {
            mbc.control(address, data);
        }
    }, mbc_);
}

uint8_t cartridge::read_ram(const address16& address) const
{
    if(!xram_enabled()) {
        return 0xFFu;
    }

    return std::visit([&](auto&& mbc) {
        return mbc.read_ram(ram_, physical_ram_addr(address));
    }, mbc_);
}

void cartridge::write_ram(const address16& address, uint8_t data)
{
    if(!xram_enabled()) {
        return;
    }

    std::visit([&](auto&& mbc) {
        mbc.write_ram(ram_, physical_ram_addr(address), data);
    }, mbc_);
}

bool cartridge::xram_enabled() const noexcept
{
    return std::visit([](auto&& mbc) {
        return mbc.xram_enabled;
    }, mbc_);
}

uint32_t cartridge::rom_bank() const noexcept
{
    return std::visit(overloaded{
        [](const mbc1& mbc) {
            const auto bank = mbc.rom_banking_active
                ? mbc.ram_bank << 0x5u | mbc.rom_bank
                : mbc.rom_bank;

            switch(bank) {
                case 0x00u:
                case 0x20u:
                case 0x40u:
                case 0x60u:
                    // these banks are unusuable, must return the next one
                    return bank + 1;
                default:
                    return bank;
            }
        },
        [](auto&& mbc) {
            return mbc.rom_bank;
        }
    }, mbc_);
}

uint32_t cartridge::ram_bank() const noexcept
{
    return std::visit(overloaded{
        [](const mbc1& mbc) {
            if(mbc.rom_banking_active) {
                return 0u;
            }

            return mbc.ram_bank;
        },
        [](auto&& mbc) {
            return mbc.ram_bank;
        }
    }, mbc_);
}

physical_address cartridge::physical_ram_addr(const address16& address) const noexcept
{
    return physical_address{address.value() - *begin(xram_range) + 8_kb * ram_bank()};
}

} // namespace gameboy
