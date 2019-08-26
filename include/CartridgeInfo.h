#ifndef GAMEBOY_CARTRIDGEINFO_H
#define GAMEBOY_CARTRIDGEINFO_H

#include <string>
#include <cstddef>

namespace gameboy {
    struct CartridgeInfo {
        enum class Type : uint8_t {
            rom_only = 0x00,
            mbc_1 = 0x01,
            mbc_1_ram = 0x02,
            mbc_1_ram_battery = 0x03,
            mbc_2 = 0x05,
            mbc_2_battery = 0x06,
            rom_ram = 0x08,
            rom_ram_battery = 0x09,
            mmm_01 = 0x0B,
            mmm_01_ram = 0x0C,
            mmm_01_ram_battery = 0x0D,
            mbc_3_timer_battery = 0x0F,
            mbc_3_timer_ram_battery = 0x10,
            mbc_3 = 0x11,
            mbc_3_ram = 0x12,
            mbc_3_ram_battery = 0x13,
            mbc_4 = 0x15,
            mbc_4_ram = 0x16,
            mbc_4_ram_battery = 0x17,
            mbc_5 = 0x19,
            mbc_5_ram = 0x1A,
            mbc_5_ram_battery = 0x1B,
            mbc_5_rumble = 0x1C,
            mbc_5_rumble_ram = 0x1D,
            mbc_5_rumble_ram_battery = 0x1E,
            pocket_camera = 0xFC,
            bandai_tama_5 = 0xFD,
            huc_3 = 0xFE,
            huc_1_ram_battery = 0xFF
        };

        enum class GameBoyColorSupport : uint8_t {
            no_support = 0x00,
            backwards_compatible = 0x80,
            only_color = 0xC0
        };

        enum class SuperGameBoySupport : uint8_t {
            no_support = 0x00,
            support_available = 0x03
        };

        enum class RomSize : uint8_t {
            kb_32 = 0x00,
            kb_64 = 0x01,
            kb_128 = 0x02,
            kb_256 = 0x03,
            kb_512 = 0x04,
            mb_1 = 0x05,
            mb_2 = 0x06,
            mb_4 = 0x07,
            mb_1_1 = 0x52,
            mb_1_2 = 0x53,
            mb_1_5 = 0x54
        };

        enum class RamSize : uint8_t {
            none = 0x00,
            kb_2 = 0x01,
            kb_8 = 0x02,
            kb_32 = 0x03
        };

        std::string name;
        GameBoyColorSupport cgb_support{GameBoyColorSupport::no_support};
        SuperGameBoySupport sgb_support{SuperGameBoySupport::no_support};
        Type cartridge_type{Type::rom_only};
        RomSize rom_size{RomSize::kb_32};
        RamSize ram_size{RamSize::none};
        bool is_checksum_correct = false;
    };
}

#endif //GAMEBOY_CARTRIDGEINFO_H
