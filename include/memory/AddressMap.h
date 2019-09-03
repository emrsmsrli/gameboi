#ifndef GAMEBOY_ADDRESSMAP_H
#define GAMEBOY_ADDRESSMAP_H

namespace gameboy::memory {
    enum Map : uint16_t {
        rom_bank_0_start = 0x0000u,
        rom_bank_0_end = 0x3FFFu,
        rom_bank_switchable_start = 0x4000u,
        rom_bank_switchable_end = 0x7FFFu,
        vram_start = 0x8000u,
        vram_end = 0x9FFFu,
        ram_external_start = 0xA000u,
        ram_external_end = 0xBFFFu,
        ram_internal_start = 0xC000u,
        ram_internal_end = 0xDFFFu,
        ram_internal_echo_start = 0xE000u,
        ram_internal_echo_end = 0xFDFFu,
        object_attribute_memory_start = 0xFE00u,
        object_attribute_memory_end = 0xFE9Fu,
        io_start = 0xFF00u,
        io_end = 0xFF4Bu,
        ram_high_start = 0xFF80u,
        ram_high_end = 0xFFFEu,
        register_interrupt_enable = 0xFFFFu,

        rom_header_begin = 0x0134u,
        rom_title_begin = rom_header_begin,
        rom_title_end = 0x0142u,
        rom_cgb_support = 0x0143u,
        rom_sgb_support = 0x0146u,
        rom_cartridge_type = 0x0147u,
        rom_rom_size = 0x0148u,
        rom_ram_size = 0x0149u,
        rom_header_end = 0x014Cu,
        rom_header_checksum = 0x014Du
    };
}

#endif //GAMEBOY_ADDRESSMAP_H
