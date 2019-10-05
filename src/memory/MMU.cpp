#include <util/RomParser.h>
#include <util/Log.h>
#include <memory/MMU.h>
#include <memory/controller/NullController.h>
#include <memory/controller/MBC1.h>
#include <memory/controller/MBC2.h>
#include <memory/controller/MBC3.h>
#include <memory/controller/MBC5.h>

void gameboy::memory::MMU::initialize()
{
    mbc->initialize();
}

void gameboy::memory::MMU::write(const gameboy::memory::Address16& address, uint8_t data)
{
    mbc->write(address, data);
}

uint8_t gameboy::memory::MMU::read(const gameboy::memory::Address16& address) const
{
    return mbc->read(address);
}

void gameboy::memory::MMU::load_rom(const std::vector<uint8_t>& rom_data)
{
    const auto rom_header = util::rom_parser::parse(rom_data);
    switch(rom_header.cartridge_type) {
        case CartridgeInfo::Type::mbc_1:
        case CartridgeInfo::Type::mbc_1_ram:
        case CartridgeInfo::Type::mbc_1_ram_battery:mbc = std::make_unique<controller::MBC1>(rom_data, rom_header);
            break;
        case CartridgeInfo::Type::mbc_2:
        case CartridgeInfo::Type::mbc_2_battery:mbc = std::make_unique<controller::MBC2>(rom_data, rom_header);
            break;
        case CartridgeInfo::Type::rom_only:
        case CartridgeInfo::Type::rom_ram:
        case CartridgeInfo::Type::rom_ram_battery:
            mbc = std::make_unique<controller::NullController>(rom_data, rom_header);
            break;
        case CartridgeInfo::Type::mbc_3_timer_battery:
        case CartridgeInfo::Type::mbc_3_timer_ram_battery:
        case CartridgeInfo::Type::mbc_3:
        case CartridgeInfo::Type::mbc_3_ram:
        case CartridgeInfo::Type::mbc_3_ram_battery:mbc = std::make_unique<controller::MBC3>(rom_data, rom_header);
            break;
        case CartridgeInfo::Type::mbc_5:
        case CartridgeInfo::Type::mbc_5_ram:
        case CartridgeInfo::Type::mbc_5_ram_battery:
        case CartridgeInfo::Type::mbc_5_rumble:
        case CartridgeInfo::Type::mbc_5_rumble_ram:
        case CartridgeInfo::Type::mbc_5_rumble_ram_battery:
            // mbc = std::make_unique<controller::MBC5>(rom_data, rom_header);
            break;
        case CartridgeInfo::Type::mbc_4:
        case CartridgeInfo::Type::mbc_4_ram:
        case CartridgeInfo::Type::mbc_4_ram_battery:
            // todo
        case CartridgeInfo::Type::mmm_01:
        case CartridgeInfo::Type::mmm_01_ram:
        case CartridgeInfo::Type::mmm_01_ram_battery:
        case CartridgeInfo::Type::pocket_camera:
        case CartridgeInfo::Type::bandai_tama_5:
        case CartridgeInfo::Type::huc_3:
        case CartridgeInfo::Type::huc_1_ram_battery:
            log::error("unimplemented cartridge type: {}", static_cast<int>(rom_header.cartridge_type));
            break;
    }
}

void gameboy::memory::MMU::load_external_memory(const std::vector<uint8_t>& save_data)
{
    // mbc->
}

std::vector<uint8_t> gameboy::memory::MMU::copy_external_memory() const
{
    return std::vector<uint8_t>(); // mbc->copy_external_memory();
    // std::vector<uint8_t> external_memory(memory::Map::ram_external_end -
    //     memory::Map::ram_external_start + 1);
    //
    // std::copy(begin(memory), end(memory))
}
