#include <spdlog/spdlog.h>

#include "gameboy/memory/mmu.h"
#include "gameboy/memory/address_range.h"
#include "gameboy/memory/memory_constants.h"
#include "gameboy/bus.h"
#include "gameboy/cartridge.h"
#include "gameboy/ppu/ppu.h"

namespace gameboy {

constexpr address16 svbk_addr{0xFF70u};

constexpr std::array<uint8_t, hram_range.size()> hram_gb{
    0x2B, 0x0B, 0x64, 0x2F, 0xAF, 0x15, 0x60, 0x6D, 0x61, 0x4E, 0xAC, 0x45, 0x0F, 0xDA, 0x92, 0xF3,
    0x83, 0x38, 0xE4, 0x4E, 0xA7, 0x6C, 0x38, 0x58, 0xBE, 0xEA, 0xE5, 0x81, 0xB4, 0xCB, 0xBF, 0x7B,
    0x59, 0xAD, 0x50, 0x13, 0x5E, 0xF6, 0xB3, 0xC1, 0xDC, 0xDF, 0x9E, 0x68, 0xD7, 0x59, 0x26, 0xF3,
    0x62, 0x54, 0xF8, 0x36, 0xB7, 0x78, 0x6A, 0x22, 0xA7, 0xDD, 0x88, 0x15, 0xCA, 0x96, 0x39, 0xD3,
    0xE6, 0x55, 0x6E, 0xEA, 0x90, 0x76, 0xB8, 0xFF, 0x50, 0xCD, 0xB5, 0x1B, 0x1F, 0xA5, 0x4D, 0x2E,
    0xB4, 0x09, 0x47, 0x8A, 0xC4, 0x5A, 0x8C, 0x4E, 0xE7, 0x29, 0x50, 0x88, 0xA8, 0x66, 0x85, 0x4B,
    0xAA, 0x38, 0xE7, 0x6B, 0x45, 0x3E, 0x30, 0x37, 0xBA, 0xC5, 0x31, 0xF2, 0x71, 0xB4, 0xCF, 0x29,
    0xBC, 0x7F, 0x7E, 0xD0, 0xC7, 0xC3, 0xBD, 0xCF, 0x59, 0xEA, 0x39, 0x01, 0x2E, 0x00, 0x69,
};

constexpr std::array<uint8_t, hram_range.size()> hram_cgb{
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
    0x45, 0xEC, 0x42, 0xFA, 0x08, 0xB7, 0x07, 0x5D, 0x01, 0xF5, 0xC0, 0xFF, 0x08, 0xFC, 0x00, 0xE5,
    0x0B, 0xF8, 0xC2, 0xCA, 0xF4, 0xF9, 0x0D, 0x7F, 0x44, 0x6D, 0x19, 0xFE, 0x46, 0x97, 0x33, 0x5E,
    0x08, 0xFF, 0xD1, 0xFF, 0xC6, 0x8B, 0x24, 0x74, 0x12, 0xFC, 0x00, 0x9F, 0x94, 0xB7, 0x06, 0xD5,
    0x40, 0x7A, 0x20, 0x9E, 0x04, 0x5F, 0x41, 0x2F, 0x3D, 0x77, 0x36, 0x75, 0x81, 0x8A, 0x70, 0x3A,
    0x98, 0xD1, 0x71, 0x02, 0x4D, 0x01, 0xC1, 0xFF, 0x0D, 0x00, 0xD3, 0x05, 0xF9, 0x00, 0x0B,
};

mmu::mmu(observer<bus> bus)
    : bus_{bus},
      wram_bank_{1u},
      work_ram_((bus->get_cartridge()->cgb_enabled() ? 8u : 2u) * 4_kb, 0u),
      high_ram_(
          (bus->get_cartridge()->cgb_enabled() ? hram_cgb : hram_gb).begin(),
          (bus->get_cartridge()->cgb_enabled() ? hram_cgb : hram_gb).end()
      ) {}

void mmu::write(const address16& address, const uint8_t data)
{
#if WITH_DEBUGGER
    if(on_write_access_) { on_write_access_(address, data); }
#endif //WITH_DEBUGGER

    if(const auto it = delegates_.find(address); it != end(delegates_)) {
        const auto& [delegated_addr, delegate] = *it;
        delegate.on_write(delegated_addr, data);
    } else if(rom_range.has(address)) {
        bus_->get_cartridge()->write_rom(address, data);
    } else if(vram_range.has(address)) {
        bus_->get_ppu()->write_ram(address, data);
    } else if(oam_range.has(address)) {
        bus_->get_ppu()->write_oam(address, data);
    } else if(xram_range.has(address)) {
        bus_->get_cartridge()->write_ram(address, data);
    } else if(echo_range.has(address)) {
        constexpr auto echo_diff = *begin(echo_range) - *begin(wram_range);
        write_wram(address - echo_diff, data);
    } else if(wram_range.has(address)) {
        write_wram(address, data);
    } else if(hram_range.has(address)) {
        write_hram(address, data);
    } else if(address == svbk_addr) {
        if(bus_->get_cartridge()->cgb_enabled()) {
            wram_bank_ = data & 0x7u;
            if(wram_bank_ == 0u) {
                wram_bank_ = 1u;
            }
        }
    } else {
        spdlog::warn("out of bounds write: {:#x}", address.value());
    }
}

uint8_t mmu::read(const address16& address) const
{
#if WITH_DEBUGGER
    if(on_read_access_) { on_read_access_(address); }
#endif //WITH_DEBUGGER

    if(const auto it = delegates_.find(address); it != end(delegates_)) {
        const auto& [delegated_addr, delegate] = *it;
        return delegate.on_read(delegated_addr);
    } 

    if(rom_range.has(address)) {
        return bus_->get_cartridge()->read_rom(address);
    } 

    if(vram_range.has(address)) {
        return bus_->get_ppu()->read_ram(address);
    } 

    if(oam_range.has(address)) {
        return bus_->get_ppu()->read_oam(address);
    } 

    if(xram_range.has(address)) {
        return bus_->get_cartridge()->read_ram(address);
    } 

    if(echo_range.has(address)) {
        constexpr auto echo_diff = *begin(echo_range) - *begin(wram_range);
        return read_wram(address - echo_diff);
    } 

    if(wram_range.has(address)) {
        return read_wram(address);
    } 

    if(hram_range.has(address)) {
        return read_hram(address);
    } 

    if(address == svbk_addr) {
        if(!bus_->get_cartridge()->cgb_enabled()) {
            return 0xFFu;
        }

        return wram_bank_ | 0xF8u;
    }

    spdlog::warn("out of bounds read: {:#x}", address.value());
    return 0xFFu;
}

void mmu::write_wram(const address16& address, const uint8_t data)
{
    work_ram_[physical_wram_addr(address).value()] = data;
}

uint8_t mmu::read_wram(const address16& address) const
{
    return work_ram_[physical_wram_addr(address).value()];
}

void mmu::write_hram(const address16& address, const uint8_t data)
{
    high_ram_[address.value() - *begin(hram_range)] = data;
}

uint8_t mmu::read_hram(const address16& address) const
{
    return high_ram_[address.value() - *begin(hram_range)];
}

physical_address mmu::physical_wram_addr(const address16& address) const noexcept
{
    static constexpr address_range wram_first_bank_range{0xC000u, 0xCFFFu};
    if(wram_first_bank_range.has(address)) {
        return physical_address(address.value() - *begin(wram_first_bank_range));
    }

    return physical_address{address.value() - *begin(wram_range) + 4_kb * (wram_bank_ - 1u)};
}

void mmu::dma(const address16& source, const address16& destination, const uint16_t length)
{
    for(uint16_t i = 0; i < length; ++i) {
        write(destination + i, read(source + i));
    }
}

} // namespace gameboy
