#include <array>
#include <memory/controller/MBC.h>
#include <memory/Address.h>
#include <memory/AddressRange.h>
#include <CartridgeInfo.h>

gameboy::memory::controller::MBC::MBC(const std::vector<uint8_t>& rom, const CartridgeInfo& rom_header)
{
    constexpr std::array<uint32_t, 11> rom_size_to_banks{2, 4, 8, 16, 32, 64, 128, 256, 72, 80, 96};

    is_cgb = rom_header.cgb_support != CartridgeInfo::GameBoyColorSupport::no_support;

    n_rom_banks = rom_size_to_banks[static_cast<std::size_t>(rom_header.rom_size)] - 1;
    n_video_ram_banks = is_cgb ? 2 : 1;
    n_external_ram_banks = rom_header.ram_size == CartridgeInfo::RamSize::kb_32 ? 4 : 1;
    n_working_ram_banks = is_cgb ? 7 : 1;

    const auto total_memory_size =
            16_kb +                         // 0000-3FFF - rom bank 0
            16_kb * n_rom_banks +           // 4000-7FFF - rom bank 1-n
            8_kb * n_video_ram_banks +      // 8000-9FFF - vram bank 0-1
            8_kb * n_external_ram_banks +   // A000-BFFF - xram bank 0-n
            4_kb +                          // C000-CFFF - wram bank 0
            4_kb * n_working_ram_banks +    // D000-DFFF - wram bank 1-7
            8_kb;                           // E000-FFFF - high ram

    memory.reserve(total_memory_size);

    std::copy(begin(rom), end(rom), std::back_inserter(memory));
}

uint8_t gameboy::memory::controller::MBC::read(const Address16& virtual_address) const
{
    if(AddressRange(0xA000u, 0xBFFFu).contains(virtual_address) && !is_external_ram_enabled) {
        return 0xFFu;
    }

    const auto physical_address = to_physical_address(virtual_address);
    return memory[physical_address.get_value()];
}

void gameboy::memory::controller::MBC::write(const Address16& virtual_address, uint8_t data)
{
    if(AddressRange(0x7FFFu).contains(virtual_address)) {
        control(virtual_address, data);
    } else {
        if(AddressRange(0xA000u, 0xBFFFu).contains(virtual_address) && !is_external_ram_enabled) {
            return;
        }

        const auto physical_address = to_physical_address(virtual_address);
        memory[physical_address.get_value()] = data;
    }
}

gameboy::memory::PhysicalAddress gameboy::memory::controller::MBC::to_physical_address(
        const Address16& virtual_address) const
{
    const auto physical_address = [&]() -> size_t {
        const size_t addr = virtual_address.get_value();
        switch(addr & 0xF000u) {
            case 0x0000u: case 0x1000u:
            case 0x2000u: case 0x3000u:
                return addr;
            case 0x4000u: case 0x5000u:
            case 0x6000u: case 0x7000u:
                return addr +
                    16_kb * get_rom_bank();
            case 0x8000u:
            case 0x9000u:
                return addr +
                    16_kb * (n_rom_banks - 1) +
                    8_kb * get_video_ram_bank();
            case 0xA000u:
            case 0xB000u:
                return addr +
                    16_kb * (n_rom_banks - 1) +
                    8_kb * (n_video_ram_banks - 1) +
                    8_kb * get_ram_bank();
            case 0xC000u:
                return addr +
                    16_kb * (n_rom_banks - 1) +
                    8_kb * (n_video_ram_banks - 1) +
                    8_kb * (n_external_ram_banks - 1);
            case 0xD000u:
                return addr +
                    16_kb * (n_rom_banks - 1) +
                    8_kb * (n_video_ram_banks - 1) +
                    8_kb * (n_external_ram_banks - 1) +
                    4_kb * get_work_ram_bank();
            case 0xE000u:
            case 0xF000u:
                return addr +
                    16_kb * (n_rom_banks - 1) +
                    8_kb * (n_video_ram_banks - 1) +
                    8_kb * (n_external_ram_banks - 1) +
                    4_kb * (n_working_ram_banks - 1);
            default:
                return 0u;
        }
    }();

    return make_address(physical_address);
}

uint32_t gameboy::memory::controller::MBC::get_video_ram_bank() const
{
    static constexpr Address16 vbk_register(0xFF4Fu);
    return read(vbk_register) & 0x1u;
}

uint32_t gameboy::memory::controller::MBC::get_work_ram_bank() const
{
    static constexpr Address16 svbk_register(0xFF70u);
    const auto bank = read(svbk_register) & 0x7u;
    return bank < 2 ? 0u : bank;
}
