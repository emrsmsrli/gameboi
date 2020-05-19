#ifndef GAMEBOY_MMU_H
#define GAMEBOY_MMU_H

#include <cstdint>
#include <vector>

#include "../../3rdparty/parallel-hashmap/parallel_hashmap/phmap.h"
#include "gameboy/memory/address.h"
#include "gameboy/util/delegate.h"
#include "gameboy/util/observer.h"

namespace gameboy {

class bus;
class cpu_debugger;
class memory_bank_debugger;
class disassembly_view;

namespace instruction {
class disassembly_db;
} // namespace instruction

struct memory_delegate {
    delegate<uint8_t(const address16&)> on_read;
    delegate<void(const address16&, uint8_t)> on_write;

    memory_delegate() noexcept = default;
    memory_delegate(
        const delegate<uint8_t(const address16&)> on_read_delegate,
        const delegate<void(const address16&, uint8_t)> on_write_delegate) noexcept
        : on_read{on_read_delegate},
          on_write{on_write_delegate} {}
};

class mmu {
    friend cpu_debugger;
    friend memory_bank_debugger;
    friend disassembly_view;
    friend instruction::disassembly_db;

public:
    explicit mmu(observer<bus> bus);

    void write(const address16& address, uint8_t data);
    [[nodiscard]] uint8_t read(const address16& address) const;

    void dma(const address16& source, const address16& destination, uint16_t length);

    void add_memory_delegate(const address16& address, const memory_delegate& callback) { delegates_[address] = callback; }

#if WITH_DEBUGGER
    void on_read_access(const delegate<void(const address16&)> on_read) noexcept { on_read_access_ = on_read; }
    void on_write_access(const delegate<void(const address16&, uint8_t)> on_write) noexcept { on_write_access_ = on_write; }
#endif //WITH_DEBUGGER

private:
    observer<bus> bus_;

    uint8_t wram_bank_;

    std::vector<uint8_t> work_ram_;
    std::vector<uint8_t> high_ram_;

    phmap::flat_hash_map<address16, memory_delegate> delegates_;

#if WITH_DEBUGGER
    delegate<void(const address16&)> on_read_access_;
    delegate<void(const address16&, uint8_t)> on_write_access_;
#endif //WITH_DEBUGGER

    void write_wram(const address16& address, uint8_t data);
    [[nodiscard]] uint8_t read_wram(const address16& address) const;
    void write_hram(const address16& address, uint8_t data);
    [[nodiscard]] uint8_t read_hram(const address16& address) const;

    [[nodiscard]] physical_address physical_wram_addr(const address16& address) const noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_MMU_H
