#ifndef GAMEBOY_MMU_H
#define GAMEBOY_MMU_H

#include <cstdint>
#include <vector>
#include <unordered_map>

#include "gameboy/memory/address.h"
#include "gameboy/util/observer.h"
#include "gameboy/util/delegate.h"

namespace gameboy {

class bus;
class memory_bank_debugger;

struct memory_delegate {
    address16 address;
    delegate<uint8_t(const address16&)> on_read;
    delegate<void(const address16&, uint8_t)> on_write;

    memory_delegate() noexcept = default;
    explicit memory_delegate(const address16 addr) noexcept
        : address{addr} {}
    memory_delegate(
        const address16 addr,
        const delegate<uint8_t(const address16&)> on_read_delegate,
        const delegate<void(const address16&, uint8_t)> on_write_delegate) noexcept
        : address{addr},
          on_read{on_read_delegate},
          on_write{on_write_delegate} {}

    bool operator==(const memory_delegate& other) const noexcept { return address == other.address; }
};

class mmu {
    friend memory_bank_debugger;

public:
    explicit mmu(observer<bus> bus);

    void write(const address16& address, uint8_t data);
    [[nodiscard]] uint8_t read(const address16& address) const;

    void dma(const address16& source, const address16& destination, uint16_t length);

    void add_memory_delegate(const memory_delegate& callback) { delegates_[callback.address] = callback; }

private:
    observer<bus> bus_;

    uint8_t wram_bank_;

    std::vector<uint8_t> work_ram_;
    std::vector<uint8_t> high_ram_;

    std::unordered_map<address16, memory_delegate> delegates_;

    void write_wram(const address16& address, uint8_t data);
    [[nodiscard]] uint8_t read_wram(const address16& address) const;
    void write_hram(const address16& address, uint8_t data);
    [[nodiscard]] uint8_t read_hram(const address16& address) const;

    [[nodiscard]] physical_address physical_wram_addr(const address16& address) const noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_MMU_H
