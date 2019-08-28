//
// Created by Emre Şimşirli on 26.08.2019.
//

#ifndef GAMEBOY_ADDRESSMAP_H
#define GAMEBOY_ADDRESSMAP_H

namespace gameboy::memory {
    enum Map : uint16_t {
        rom_bank_0_start = 0x0000,
        rom_bank_0_end = 0x3FFF,
        rom_bank_switchable_start = 0x4000,
        rom_bank_switchable_end = 0x7FFF,
        vram_start = 0x8000,
        vram_end = 0x9FFF,
        ram_external_start = 0xA000,
        ram_external_end = 0xBFFF,
        ram_internal_start = 0xC000,
        ram_internal_end = 0xDFFF,
        ram_internal_echo_start = 0xE000,
        ram_internal_echo_end = 0xFDFF,
        object_attribute_memory_start = 0xFE00,
        object_attribute_memory_end = 0xFE9F,
        io_start = 0xFF00,
        io_end = 0xFF4B,
        ram_high_start = 0xFF80,
        ram_high_end = 0xFFFE,
        register_interrupt_enable = 0xFFFF,

        rom_header_begin = 0x0134,
        rom_title_begin = rom_header_begin,
        rom_title_end = 0x0142,
        rom_cgb_support = 0x0143,
        rom_sgb_support = 0x0146,
        rom_cartridge_type = 0x0147,
        rom_rom_size = 0x0148,
        rom_ram_size = 0x0149,
        rom_header_end = 0x014C,
        rom_header_checksum = 0x014D,
    };
}

#endif //GAMEBOY_ADDRESSMAP_H
