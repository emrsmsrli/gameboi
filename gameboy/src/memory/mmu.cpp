#include <algorithm>
#include <map>

#include "gameboy/util/log.h"
#include "gameboy/memory/mmu.h"
#include "gameboy/memory/address_range.h"
#include "gameboy/memory/memory_constants.h"
#include "gameboy/bus.h"
#include "gameboy/cartridge.h"
#include "gameboy/ppu/ppu.h"

namespace gameboy {

constexpr address16 svbk_addr{0xFF70u};

template<typename T>
auto find_callback(const std::vector<T>& container, T&& value) noexcept
{
    return std::find(begin(container), end(container), std::forward<T>(value));
}

mmu::mmu(observer<bus> bus)
    : bus_{bus},
      wram_bank_{0u},
      work_ram_((bus->get_cartridge()->cgb_enabled() ? 8u : 2u) * 4_kb, 0u),
      high_ram_(hram_range.size(), 0u) {}

void mmu::write(const address16& address, const uint8_t data)
{
    if(const auto it = delegates_.find(address); it != end(delegates_)) {
        const auto& [addr, del] = *it;
        del.on_write(address, data);
    } else if(rom_range.has(address)) {
        bus_->get_cartridge()->write_rom(address, data);
    } else if(vram_range.has(address) || oam_range.has(address)) {
        bus_->get_ppu()->write(address, data);
    } else if(xram_range.has(address)) {
        bus_->get_cartridge()->write_ram(address, data);
    } else if(echo_range.has(address)) {
        write_wram(address16(address.value() - 0x1000u), data);
    } else if(wram_range.has(address)) {
        write_wram(address, data);
    } else if(hram_range.has(address)) {
        write_hram(address, data);
    } else if(address == svbk_addr) {
        wram_bank_ = data & 0x7u;
    } else {
        log::info("out of bounds address: {:#x}", address.value());
    }
}

uint8_t mmu::read(const address16& address) const
{
    if(const auto it = delegates_.find(address); it != end(delegates_)) {
        const auto& [addr, del] = *it;
        return del.on_read(address);
    } 

    if(rom_range.has(address)) {
        return bus_->get_cartridge()->read_rom(address);
    } 

    if(vram_range.has(address) || oam_range.has(address)) {
        return bus_->get_ppu()->read(address);
    } 

    if(xram_range.has(address)) {
        return bus_->get_cartridge()->read_ram(address);
    } 

    if(echo_range.has(address)) {
        return read_wram(address16(address.value() - 0x1000u));
    } 

    if(wram_range.has(address)) {
        return read_wram(address);
    } 

    if(hram_range.has(address)) {
        return read_hram(address);
    } 

    if(address == svbk_addr) {
        return wram_bank_;
    } 

    log::error("out of bounds address: {:#x}", address.value());
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
    const auto wram_bank = wram_bank_ < 2 ? 0u : wram_bank_;
    return physical_address(address.value() - *begin(wram_range) + 4_kb * wram_bank);
}

void mmu::dma(const address16& source, const address16& destination, const uint16_t length)
{
    for(uint16_t i = 0; i < length; ++i) {
        write(destination + i, read(source + i));
    }
}

} // namespace gameboy
