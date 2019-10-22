#ifndef GAMEBOY_MMU_H
#define GAMEBOY_MMU_H

#include <cstdint>
#include <vector>
#include <memory>

#include <memory/controller/mbc.h>
#include <memory/address.h>
#include <util/observer.h>
#include <util/delegate.h>

namespace gameboy {

class bus;

struct memory_callback {
    address16 address;
    delegate<uint8_t(const address16&)> on_read;
    delegate<void(const address16&, uint8_t)> on_write;

    memory_callback() noexcept = default;
    explicit memory_callback(address16 addr) noexcept
        : address(addr) {}

    bool operator==(const memory_callback& other) const noexcept { return address == other.address; }
};

class mmu {
public:
    explicit mmu(observer<bus> bus);

    void initialize();

    void write(const address16& address, uint8_t data);
    [[nodiscard]] uint8_t read(const address16& address) const;

    void add_memory_callback(const memory_callback& callback) { callbacks_.push_back(callback); }

private:
    observer<bus> bus_;

    uint8_t wram_bank_ = 0;

    std::vector<uint8_t> work_ram_;
    std::vector<uint8_t> high_ram_;

    std::vector<memory_callback> callbacks_;

    void write_wram(const address16& address, uint8_t data);
    [[nodiscard]] uint8_t read_wram(const address16& address) const;

    [[nodiscard]] physical_address physical_wram_addr(const address16& address) const noexcept;
};

}

#endif //GAMEBOY_MMU_H
