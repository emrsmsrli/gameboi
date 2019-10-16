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

struct bus;

struct on_read_callback {
    address16 address;
    delegate<uint8_t(const address16&)> on_read;

    on_read_callback() = default;
    explicit on_read_callback(address16 addr)
        : address(addr) {}

    bool operator==(const on_read_callback& other) const noexcept { return address == other.address; }
};

struct on_write_callback {
    address16 address;
    delegate<void(const address16&, uint8_t)> on_write;

    on_write_callback() = default;
    explicit on_write_callback(address16 addr)
        : address(addr) {}

    bool operator==(const on_write_callback& other) const noexcept { return address == other.address; }
};

class mmu {
public:
    explicit mmu(observer<bus> bus);

    void initialize();

    void write(const address16& address, uint8_t data);
    [[nodiscard]] uint8_t read(const address16& address) const;

    void add_read_callback(on_read_callback callback) { on_read_callbacks_.push_back(callback); }
    void add_write_callback(on_write_callback callback) { on_write_callbacks_.push_back(callback); }

private:
    observer<bus> bus_;

    uint8_t wram_bank_ = 0;

    std::vector<uint8_t> work_ram_;
    std::vector<uint8_t> high_ram_;

    std::vector<on_read_callback> on_read_callbacks_;
    std::vector<on_write_callback> on_write_callbacks_;

    void write_wram(const address16& address, uint8_t data);
    [[nodiscard]] uint8_t read_wram(const address16& address) const;
};

}

#endif //GAMEBOY_MMU_H
