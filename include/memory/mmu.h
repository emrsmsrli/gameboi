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

struct read_callback {
    address16 address;
    delegate<uint8_t(const address16&)> on_read;

    read_callback() noexcept = default;
    explicit read_callback(address16 addr) noexcept
        : address(addr) {}

    bool operator==(const read_callback& other) const noexcept { return address == other.address; }
};

struct write_callback {
    address16 address;
    delegate<void(const address16&, uint8_t)> on_write;

    write_callback() noexcept = default;
    explicit write_callback(address16 addr) noexcept
        : address(addr) {}

    bool operator==(const write_callback& other) const noexcept { return address == other.address; }
};

class mmu {
public:
    explicit mmu(observer<bus> bus);

    void initialize();

    void write(const address16& address, uint8_t data);
    [[nodiscard]] uint8_t read(const address16& address) const;

    void add_read_callback(read_callback callback) { read_callbacks_.push_back(callback); }
    void add_write_callback(write_callback callback) { write_callbacks_.push_back(callback); }

private:
    observer<bus> bus_;

    uint8_t wram_bank_ = 0;

    std::vector<uint8_t> work_ram_;
    std::vector<uint8_t> high_ram_;

    std::vector<read_callback> read_callbacks_;
    std::vector<write_callback> write_callbacks_;

    void write_wram(const address16& address, uint8_t data);
    [[nodiscard]] uint8_t read_wram(const address16& address) const;

    [[nodiscard]] physical_address physical_wram_addr(const address16& address) const noexcept;
};

}

#endif //GAMEBOY_MMU_H
