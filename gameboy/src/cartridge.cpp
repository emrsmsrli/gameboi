#include "gameboy/cartridge.h"

#include <numeric>

#include <spdlog/spdlog.h>

#include "gameboy/memory/address_range.h"
#include "gameboy/memory/memory_constants.h"
#include "gameboy/util/mathutil.h"
#include "gameboy/util/variantutil.h"

namespace gameboy {

constexpr address_range first_rom_bank_range{0x3FFFu};

enum class rom_type : uint8_t {
    kb_32 = 0x00u,
    kb_64 = 0x01u,
    kb_128 = 0x02u,
    kb_256 = 0x03u,
    kb_512 = 0x04u,
    mb_1 = 0x05u,
    mb_2 = 0x06u,
    mb_4 = 0x07u,
    mb_8 = 0x08u,
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

filesystem::path get_external_path(const filesystem::path& rom_path, const filesystem::path& ext) noexcept
{
    auto save_path = rom_path;
    save_path.replace_extension(ext);
    return save_path;
}

filesystem::path get_save_path(const filesystem::path& rom_path) noexcept
{
    return get_external_path(rom_path, ".sav");
}

filesystem::path get_rtc_path(const filesystem::path& rom_path) noexcept
{
    return get_external_path(rom_path, ".rtc");
}

cartridge::cartridge(const filesystem::path& rom_path)
    : rom_path_{rom_path},
      rom_{read_file(rom_path)},
      mbc_{mbc_regular{make_observer(this)}}
{
    parse_rom();
}

void cartridge::parse_rom()
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
        std::terminate();
    }

    name_.clear();
    std::copy(
        begin(rom_) + *begin(rom_title_range),
        begin(rom_) + *end(rom_title_range),
        std::back_inserter(name_));

    const auto cgb_flag = read<uint8_t>(rom_, cgb_support_addr);
    cgb_enabled_ = bit::test(cgb_flag, 7u) && !(bit::test(cgb_flag, 2u) || bit::test(cgb_flag, 3u));

    has_rtc_ = false;

    const auto mbc = read<mbc_type>(rom_, mbc_type_addr);
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
            has_rtc_ = mbc == mbc_type::mbc_3_rtc_battery || mbc == mbc_type::mbc_3_rtc_ram_battery;
            mbc_ = mbc3{make_observer(this), load_rtc()};
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
            spdlog::critical("unimplemented cartridge type {:#04x}", static_cast<int>(mbc));
            std::terminate();
        }
    }

    const auto rom_size_type = read<rom_type>(rom_, rom_size_addr);
    rom_bank_count_ = [](rom_type type) {
        switch(type) {
            case rom_type::kb_32:   return 2u;
            case rom_type::kb_64:   return 4u;
            case rom_type::kb_128:  return 8u;
            case rom_type::kb_256:  return 16u;
            case rom_type::kb_512:  return 32u;
            case rom_type::mb_1:    return 64u;
            case rom_type::mb_2:    return 128u;
            case rom_type::mb_4:    return 256u;
            case rom_type::mb_8:    return 512u;
            case rom_type::mb_1_1:  return 72u;
            case rom_type::mb_1_2:  return 80u;
            case rom_type::mb_1_5:  return 96u;
        }
    }(rom_size_type);

    const auto ram_size_type = read<ram_type>(rom_, ram_size_addr);
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
            has_battery_ = false;
            break;
    }

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

void cartridge::load_rom(const filesystem::path& rom_path)
{
    rom_path_ = rom_path;
    rom_ = read_file(rom_path);
    parse_rom();
}

void cartridge::save_ram_rtc() const
{
    spdlog::trace("saving ram and rtc data");

    save_ram();
    save_rtc();
}

uint8_t cartridge::read_rom(const address16& address) const
{
    const auto physical_addr = [&]() -> size_t {
        if(first_rom_bank_range.has(address)) {
            return address.value() + 16_kb * rom_bank(address);
        }

        return address.value() + 16_kb * (static_cast<int32_t>(rom_bank(address)) - 1);
    }();

    return rom_[physical_addr];
}

void cartridge::write_rom(const address16& address, uint8_t data)
{
    visit_nt(mbc_,
        [](mbc_regular&) {},
        [&](auto&& mbc) {
            mbc.control(address, data);
        }
    );
}

uint8_t cartridge::read_ram(const address16& address) const
{
    if(!ram_enabled()) {
        return 0xFFu;
    }

    return visit_nt(mbc_, [&](auto&& mbc) {
        return mbc.read_ram(physical_ram_addr(address));
    });
}

void cartridge::write_ram(const address16& address, uint8_t data)
{
    if(!ram_enabled()) {
        return;
    }

    visit_nt(mbc_, [&](auto&& mbc) {
        mbc.write_ram(physical_ram_addr(address), data);
    });
}

bool cartridge::ram_enabled() const noexcept
{
    return visit_nt(mbc_, [](auto&& mbc) {
        return mbc.is_ram_enabled();
    });
}

uint32_t cartridge::rom_bank(const address16& address) const noexcept
{
   return visit_nt(mbc_,
        [&](const mbc1& mbc) {
            if(first_rom_bank_range.has(address)) {
                if(mbc.rom_banking_active()) {
                    return 0u;
                }

                return mbc.ram_bank() << 5u;
            }

            return mbc.rom_bank() | (mbc.ram_bank() << 5u);
        },
        [&](auto&& mbc) {
            if(first_rom_bank_range.has(address)) {
                return 0u;
            }

            return mbc.rom_bank();
        }
    ) & (rom_bank_count() - 1u);
}

uint32_t cartridge::ram_bank() const noexcept
{
    return visit_nt(mbc_,
        [](const mbc1& mbc) {
            if(mbc.rom_banking_active()) {
                return 0u;
            }

            return mbc.ram_bank();
        },
        [](auto&& mbc) {
            return mbc.ram_bank();
        }
    ) & (ram_bank_count() - 1u);
}

physical_address cartridge::physical_ram_addr(const address16& address) const noexcept
{
    return physical_address{address.value() - *begin(xram_range) + 8_kb * ram_bank()};
}

void cartridge::load_ram()
{
    const auto save_path = get_save_path(rom_path_);
    if(filesystem::exists(save_path) && filesystem::file_size(save_path) == ram_.size()) {
        ram_ = read_file(save_path);
    }
}

void cartridge::save_ram() const
{
    if(has_battery()) {
        write_file(get_save_path(rom_path_), ram_);
    }
}

std::pair<std::time_t, rtc> cartridge::load_rtc()
{
    if(const auto rtc_path = get_rtc_path(rom_path_); has_rtc() && filesystem::exists(rtc_path)) {
        // todo use std::bit_cast in c++20
        const auto rtc_data = read_file(rtc_path);

        std::time_t rtc_last_time;
        rtc rtc;

        std::memcpy(&rtc_last_time, rtc_data.data(), sizeof(rtc_last_time));
        std::memcpy(&rtc, rtc_data.data() + sizeof(rtc_last_time), sizeof(rtc));

        return std::make_pair(rtc_last_time, rtc);
    }

    return std::make_pair(0u, rtc{});
}

void cartridge::save_rtc() const
{
    if(has_rtc()) {
        const auto mbc = std::get<mbc3>(mbc_);
        // todo use std::bit_cast in c++20
        const auto [rtc_last_time, rtc] = mbc.get_rtc_data();

        std::vector<uint8_t> rtc_data(sizeof(rtc_last_time) + sizeof(rtc));

        std::memcpy(rtc_data.data(), &rtc_last_time, sizeof(rtc_last_time));
        std::memcpy(rtc_data.data() + sizeof(rtc_last_time), &rtc, sizeof(rtc));

        write_file(get_rtc_path(rom_path_), rtc_data);
    }
}

} // namespace gameboy
