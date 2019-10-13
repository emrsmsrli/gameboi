#include <algorithm>
#include <map>

#include <util/log.h>
#include <memory/mmu.h>
#include <memory/address_range.h>
#include <memory/memory_constants.h>
#include <bus.h>
#include <cartridge.h>
#include <ppu/ppu.h>

gameboy::mmu::mmu(observer<bus> bus)
    : bus_(bus)
{
    work_ram_.reserve((bus->cartridge->cgb_enabled() ? 8 : 2) * 4_kb);
    high_ram_.reserve(1_kb); // fixme probably less than this

    std::fill(begin(work_ram_), end(work_ram_), 0u);
    std::fill(begin(high_ram_), end(high_ram_), 0u);
}

void gameboy::mmu::initialize()
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

void gameboy::mmu::write(const gameboy::address16& address, const uint8_t data)
{
    if(rom_range.contains(address)) {
        bus_->cartridge->write_rom(address, data);
    } else if(vram_range.contains(address)) {
        bus_->ppu->write(address, data);
    } else if(xram_range.contains(address)) {
        bus_->cartridge->write_ram(address, data);
    } else if(wram_range.contains(address)) {
        // todo also check echo range 0xE000-0xFDFF   Same as C000-DDFF
        // fixme get correct wram bank
        work_ram_[address.value()] = data;
    }
    // todo switch here baby
}

uint8_t gameboy::mmu::read(const gameboy::address16& address) const
{
    if(rom_range.contains(address)) {
        return bus_->cartridge->read_rom(address);
    } else if(vram_range.contains(address)) {
        return bus_->ppu->read(address);
    } else if(xram_range.contains(address)) {
        return bus_->cartridge->read_ram(address);
    } else if(wram_range.contains(address)) {
        // todo also check echo range 0xE000-0xFDFF   Same as C000-DDFF
        // fixme get correct wram bank
        return work_ram_[address.value()];
    } else {
        log::error("out of bounds address: {}", address.value());
    }
    // todo switch here baby
}
