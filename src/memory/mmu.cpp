#include <algorithm>
#include <map>

#include <util/log.h>
#include <memory/mmu.h>
#include <memory/address_range.h>
#include <memory/memory_constants.h>
#include <bus.h>
#include <cartridge.h>
#include <ppu/ppu.h>

namespace gameboy {

constexpr address16 svbk_addr(0xFF70u);

template<typename T>
auto find_callback(const std::vector<T>& container, T&& value) noexcept
{
    return std::find(begin(container), end(container), std::forward<T>(value));
}

mmu::mmu(observer<bus> bus)
    : bus_(bus),
      work_ram_((bus->cartridge->cgb_enabled() ? 8 : 2) * 4_kb, 0u),
      high_ram_(hram_range.high() - hram_range.low() + 1, 0u) {}

void mmu::initialize()
{
    std::map<uint16_t, uint8_t> initialization_sequence{
        {0xFF05u, 0x00u}, // TIMA
        {0xFF06u, 0x00u}, // TMA
        {0xFF07u, 0x00u}, // TAC
        {0xFF10u, 0x80u}, // NR10
        {0xFF11u, 0xBFu}, // NR11
        {0xFF12u, 0xF3u}, // NR12
        {0xFF14u, 0xBFu}, // NR14
        {0xFF16u, 0x3Fu}, // NR21
        {0xFF17u, 0x00u}, // NR22
        {0xFF19u, 0xBFu}, // NR24
        {0xFF1Au, 0x7Fu}, // NR30
        {0xFF1Bu, 0xFFu}, // NR31
        {0xFF1Cu, 0x9Fu}, // NR32
        {0xFF1Eu, 0xBFu}, // NR33
        {0xFF20u, 0xFFu}, // NR41
        {0xFF21u, 0x00u}, // NR42
        {0xFF22u, 0x00u}, // NR43
        {0xFF23u, 0xBFu}, // NR30
        {0xFF24u, 0x77u}, // NR50
        {0xFF25u, 0xF3u}, // NR51
        {0xFF26u, 0xF1u}, // NR52
        {0xFF40u, 0x91u}, // LCDC
        {0xFF42u, 0x00u}, // SCY
        {0xFF43u, 0x00u}, // SCX
        {0xFF45u, 0x00u}, // LYC
        {0xFF47u, 0xFCu}, // BGP
        {0xFF48u, 0xFFu}, // OBP0
        {0xFF49u, 0xFFu}, // OBP1
        {0xFF4Au, 0x00u}, // WY
        {0xFF4Bu, 0x00u}, // WX
        {0xFFFFu, 0x00u}  // IE
    };

    for(const auto&[addr, default_value] : initialization_sequence) {
        write(make_address(addr), default_value);
    }
}

void mmu::write(const address16& address, const uint8_t data)
{
    if(auto it = find_callback(write_callbacks_, write_callback{address}); it != end(write_callbacks_)) {
        (*it).on_write(address, data);
    } else if(rom_range.contains(address)) {
        bus_->cartridge->write_rom(address, data);
    } else if(vram_range.contains(address)) {
        bus_->ppu->write(address, data);
    } else if(xram_range.contains(address)) {
        bus_->cartridge->write_ram(address, data);
    } else if(echo_range.contains(address)) {
        write_wram(address16(address.value() - 0x1000u), data);
    } else if(wram_range.contains(address)) {
        write_wram(address, data);
    } else if(address == svbk_addr) {
        wram_bank_ = data & 0x7u;
    } else {
        log::error("out of bounds address: {:x}", address.value());
    }
}

uint8_t mmu::read(const address16& address) const
{
    if(auto it = find_callback(read_callbacks_, read_callback{address}); it != end(read_callbacks_)) {
        return (*it).on_read(address);
    } else if(rom_range.contains(address)) {
        return bus_->cartridge->read_rom(address);
    } else if(vram_range.contains(address)) {
        return bus_->ppu->read(address);
    } else if(xram_range.contains(address)) {
        return bus_->cartridge->read_ram(address);
    } else if(echo_range.contains(address)) {
        return read_wram(address16(address.value() - 0x1000u));
    } else if(wram_range.contains(address)) {
        return read_wram(address);
    } else if(address == svbk_addr) {
        return wram_bank_;
    } else {
        log::error("out of bounds address: {:x}", address.value());
    }
}

void mmu::write_wram(const address16& address, const uint8_t data)
{
    work_ram_[physical_wram_addr(address).value()] = data;
}

uint8_t mmu::read_wram(const address16& address) const
{
    return work_ram_[physical_wram_addr(address).value()];
}

physical_address mmu::physical_wram_addr(const address16& address) const noexcept
{
    const auto wram_bank = wram_bank_ < 2 ? 0u : wram_bank_;
    return physical_address(address.value() - wram_range.low() + 4_kb * wram_bank);
}

} // namespace gameboy
