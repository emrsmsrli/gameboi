#include <util/rom_parser.h>
#include <util/log.h>
#include <memory/mmu.h>
#include <memory/controller/mbc_null.h>
#include <memory/controller/mbc1.h>
#include <memory/controller/mbc2.h>
#include <memory/controller/mbc3.h>

void gameboy::mmu::initialize() const
{
    mbc_->initialize();
}

void gameboy::mmu::write(const gameboy::address16& address, const uint8_t data) const
{
    mbc_->write(address, data);
}

uint8_t gameboy::mmu::read(const gameboy::address16& address) const
{
    return mbc_->read(address);
}

void gameboy::mmu::load_rom(const std::vector<uint8_t>& rom_data)
{
    const auto rom_header = util::rom_parser::parse(rom_data);
    switch(rom_header.cartridge_type_) {
        case cartridge::type::mbc_1:
        case cartridge::type::mbc_1_ram:
        case cartridge::type::mbc_1_ram_battery: {
            mbc_ = std::make_unique<mbc1>(rom_data, rom_header);
            break;
        }
        case cartridge::type::mbc_2:
        case cartridge::type::mbc_2_battery:
            mbc_ = std::make_unique<mbc2>(rom_data, rom_header);
            break;
        case cartridge::type::rom_only:
        case cartridge::type::rom_ram:
        case cartridge::type::rom_ram_battery:
            mbc_ = std::make_unique<mbc_null>(rom_data, rom_header);
            break;
        case cartridge::type::mbc_3_timer_battery:
        case cartridge::type::mbc_3_timer_ram_battery:
        case cartridge::type::mbc_3:
        case cartridge::type::mbc_3_ram:
        case cartridge::type::mbc_3_ram_battery:
            mbc_ = std::make_unique<mbc3>(rom_data, rom_header);
            break;
        case cartridge::type::mbc_5:
        case cartridge::type::mbc_5_ram:
        case cartridge::type::mbc_5_ram_battery:
        case cartridge::type::mbc_5_rumble:
        case cartridge::type::mbc_5_rumble_ram:
        case cartridge::type::mbc_5_rumble_ram_battery:
            // mbc = std::make_unique<controller::MBC5>(rom_data, rom_header);
            break;
        case cartridge::type::mbc_4:
        case cartridge::type::mbc_4_ram:
        case cartridge::type::mbc_4_ram_battery:
            // todo
        case cartridge::type::mmm_01:
        case cartridge::type::mmm_01_ram:
        case cartridge::type::mmm_01_ram_battery:
        case cartridge::type::pocket_camera:
        case cartridge::type::bandai_tama_5:
        case cartridge::type::huc_3:
        case cartridge::type::huc_1_ram_battery:
            log::error("unimplemented cartridge type: {}", static_cast<int>(rom_header.cartridge_type_));
            break;
    }
}

void gameboy::mmu::load_external_memory(const std::vector<uint8_t>& save_data)
{
    // mbc->
}

std::vector<uint8_t> gameboy::mmu::copy_external_memory() const
{
    return std::vector<uint8_t>(); // mbc->copy_external_memory();
    // std::vector<uint8_t> external_memory(Map::ram_external_end -
    //     Map::ram_external_start + 1);
    //
    // std::copy(begin(memory), end(memory))
}
