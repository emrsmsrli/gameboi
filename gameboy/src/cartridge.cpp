#include <algorithm>
#include <numeric>

#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "gameboy/cartridge.h"
#include "gameboy/memory/address.h"
#include "gameboy/memory/address_range.h"
#include "gameboy/memory/memory_constants.h"
#include "gameboy/util/overloaded.h"

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
    mb_4 = 0x07u,
    mb_1_1 = 0x52u,
    mb_1_2 = 0x53u,
    mb_1_5 = 0x54u
};

enum class ram_type : uint8_t {
    none = 0x00u,
    kb_2 = 0x01u,
    kb_8 = 0x02u,
    kb_32 = 0x03u,
    kb_128 = 0x04u,
    kb_64 = 0x05u
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
    mbc_3_rtc_battery = 0x0Fu,
    mbc_3_rtc_ram_battery = 0x10u,
    mbc_3 = 0x11u,
    mbc_3_ram = 0x12u,
    mbc_3_ram_battery = 0x13u,
    mbc_5 = 0x19u,
    mbc_5_ram = 0x1Au,
    mbc_5_ram_battery = 0x1Bu,
    mbc_5_rumble = 0x1Cu,
    mbc_5_rumble_ram = 0x1Du,
    mbc_5_rumble_ram_battery = 0x1Eu,
    mbc_6 = 0x20u,
    mbc_7_sensor_rumble_ram_battery = 0x22u,
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

filesystem::path get_save_path(const filesystem::path& rom_path) noexcept
{
    auto save_path = rom_path;
    save_path.replace_extension(".sav");
    return save_path;
}

cartridge::cartridge(const filesystem::path& rom_path)
    : rom_path_{rom_path},
      rom_{load_file(rom_path)},
      mbc_{mbc_regular{make_observer(this)}}
{
    constexpr auto cgb_support_addr = make_address(0x0143u);
    constexpr auto mbc_type_addr = make_address(0x0147u);
    constexpr auto rom_size_addr = make_address(0x0148u);
    constexpr auto ram_size_addr = make_address(0x0149u);
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
        spdlog::critical("rom checksum is not correct. expected: {}, calculated: {}", expected, checksum);
    }

    std::copy(
        begin(rom_) + *begin(rom_title_range),
        begin(rom_) + *end(rom_title_range),
        std::back_inserter(name_));

    const auto cgb = read<cgb_type>(rom_, cgb_support_addr);
    cgb_enabled_ = cgb != cgb_type::only_gb;
    cgb_type_ = magic_enum::enum_name(cgb);

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
            mbc_ = mbc1{make_observer(this)};
            break;
        }
        case mbc_type::mbc_2:
        case mbc_type::mbc_2_battery: {
            mbc_ = mbc2{make_observer(this)};
            break;
        }
        case mbc_type::mbc_3_rtc_battery:
        case mbc_type::mbc_3_rtc_ram_battery:
        case mbc_type::mbc_3:
        case mbc_type::mbc_3_ram:
        case mbc_type::mbc_3_ram_battery: {
            mbc_ = mbc3{make_observer(this)};
            break;
        }
        case mbc_type::mbc_5:
        case mbc_type::mbc_5_ram:
        case mbc_type::mbc_5_ram_battery:
        case mbc_type::mbc_5_rumble:
        case mbc_type::mbc_5_rumble_ram:
        case mbc_type::mbc_5_rumble_ram_battery: {
            mbc_ = mbc5{make_observer(this)};
            break;
        }
        default: {
            spdlog::critical("unimplemented cartridge type {:#04x}", magic_enum::enum_integer(mbc));
        }
    }

    const auto rom_size_type = read<rom_type>(rom_, rom_size_addr);
    rom_type_ = magic_enum::enum_name(rom_size_type);
    rom_bank_count_ = [](rom_type type) {
        switch(type) {
            case rom_type::kb_32:   return 0u;
            case rom_type::kb_64:   return 4u;
            case rom_type::kb_128:  return 8u;
            case rom_type::kb_256:  return 16u;
            case rom_type::kb_512:  return 32u;
            case rom_type::mb_1:    return 64u;
            case rom_type::mb_2:    return 128u;
            case rom_type::mb_4:    return 256u;
            case rom_type::mb_1_1:  return 72u;
            case rom_type::mb_1_2:  return 80u;
            case rom_type::mb_1_5:  return 96u;
        }
    }(rom_size_type);

    const auto ram_size_type = read<ram_type>(rom_, ram_size_addr);
    ram_type_ = magic_enum::enum_name(ram_size_type);
    ram_bank_count_ = [](ram_type type) {
        switch(type) {
            case ram_type::none:    return 0u;
            case ram_type::kb_2:
            case ram_type::kb_8:    return 1u;
            case ram_type::kb_32:   return 4u;
            case ram_type::kb_128:  return 16u;
            case ram_type::kb_64:   return 8u;
        }
    }(ram_size_type);

    const auto allocate_ram = [&](size_t amount) {
        ram_.resize(amount);
        std::fill(begin(ram_), end(ram_), 0u);
    };

    if(mbc == mbc_type::mbc_2 || mbc == mbc_type::mbc_2_battery) { // has 512 * 4 bits of ram
        allocate_ram(512u);
    } else if(ram_bank_count_ != 0u) {
        allocate_ram(ram_bank_count_ * 8_kb);
    }

    switch(mbc) {
        case mbc_type::mbc_1_ram_battery:
        case mbc_type::mbc_2_battery:
        case mbc_type::rom_ram_battery:
        case mbc_type::mmm_01_ram_battery:
        case mbc_type::mbc_3_rtc_battery:
        case mbc_type::mbc_3_rtc_ram_battery:
        case mbc_type::mbc_3_ram_battery:
        case mbc_type::mbc_5_ram_battery:
        case mbc_type::mbc_5_rumble_ram_battery:
        case mbc_type::huc_1_ram_battery:
        case mbc_type::mbc_7_sensor_rumble_ram_battery:
            has_battery_ = true;
            load_ram();
            break;
        default:
            break;
    }

    has_rtc_ = mbc == mbc_type::mbc_3_rtc_battery || mbc == mbc_type::mbc_3_rtc_ram_battery;

    spdlog::info("----- cartridge -----");
    spdlog::info("name: {}", name_);
    spdlog::info("type: {}", mbc_type_);
    spdlog::info("cgb: {}, {}", cgb_enabled_, cgb_type_);
    spdlog::info("battery: {}", has_battery());
    spdlog::info("rtc: {}", has_rtc());
    spdlog::info("rom: {}", rom_type_);
    spdlog::info("ram: {}", ram_type_);
    spdlog::info("---------------------\n");
}

cartridge::~cartridge()
{
    save_ram();
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
    if(!ram_enabled()) {
        return 0xFFu;
    }

    return std::visit([&](auto&& mbc) {
        return mbc.read_ram(physical_ram_addr(address));
    }, mbc_);
}

void cartridge::write_ram(const address16& address, uint8_t data)
{
    if(!ram_enabled()) {
        return;
    }

    std::visit([&](auto&& mbc) {
        mbc.write_ram(physical_ram_addr(address), data);
    }, mbc_);
}

bool cartridge::ram_enabled() const noexcept
{
    return std::visit([](auto&& mbc) {
        return mbc.is_ram_enabled();
    }, mbc_);
}

uint32_t cartridge::rom_bank() const noexcept
{
    return std::visit([](auto&& mbc) {
        return mbc.rom_bank();
    }, mbc_);
}

uint32_t cartridge::ram_bank() const noexcept
{
    return std::visit(overloaded{
        [](const mbc1& mbc) {
            if(mbc.rom_banking_active()) {
                return 0u;
            }

            return mbc.ram_bank();
        },
        [](auto&& mbc) {
            return mbc.ram_bank();
        }
    }, mbc_);
}

physical_address cartridge::physical_ram_addr(const address16& address) const noexcept
{
    return physical_address{address.value() - *begin(xram_range) + 8_kb * ram_bank()};
}

void cartridge::load_ram()
{
    const auto save_path = get_save_path(rom_path_);
    if(filesystem::exists(save_path) && filesystem::file_size(save_path) == ram_.size()) {
        ram_ = load_file(save_path);
    }
}

void cartridge::save_ram()
{
    if(has_battery()) {
        write_file(get_save_path(rom_path_), ram_);
    }
}

} // namespace gameboy
