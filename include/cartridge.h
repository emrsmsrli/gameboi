#ifndef GAMEBOY_CARTRIDGE_H
#define GAMEBOY_CARTRIDGE_H

#include <string>
#include <cstddef>

namespace gameboy {
    struct cartridge {
        enum class type : uint8_t {
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

        enum class gbc_support : uint8_t {
            no_support = 0x00u,
            backwards_compatible = 0x80u,
            only_color = 0xC0u
        };

        enum class sgb_support : uint8_t {
            no_support = 0x00,
            support_available = 0x03
        };

        enum class rom_size : uint8_t {
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

        enum class ram_size : uint8_t {
            none = 0x00u,
            kb_2 = 0x01u,
            kb_8 = 0x02u,
            kb_32 = 0x03u
        };

        std::string name_;
        gbc_support cgb_support_{gbc_support::no_support};
        sgb_support sgb_support_{sgb_support::no_support};
        type cartridge_type_{type::rom_only};
        rom_size rom_size_{rom_size::kb_32};
        ram_size ram_size_{ram_size::none};
    };
}

#endif //GAMEBOY_CARTRIDGE_H
