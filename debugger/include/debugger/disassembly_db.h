#ifndef GAMEBOY_DISASSEMBLY_DB_H
#define GAMEBOY_DISASSEMBLY_DB_H

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "debugger/disassembly.h"
#include "gameboy/util/observer.h"
#include "gameboy/memory/address.h"

namespace gameboy {

class bus;

namespace instruction {

class disassembly_db {
public:
    static constexpr std::string_view name_rom = "ROM";
    static constexpr std::string_view name_wram = "WRM";
    static constexpr std::string_view name_hram = "HRM";

    disassembly_db(observer<bus> bus, std::string_view name, const std::vector<uint8_t>& data) noexcept;

    void on_write(const address16& addr, uint8_t data) noexcept;
    [[nodiscard]] const std::vector<disassembly>& get() noexcept;

private:
    observer<bus> bus_;
    const std::vector<uint8_t>& data_;
    std::vector<disassembly> disassemblies_;
    std::string_view name_;
    size_t bank_size_ = 0u;
    std::optional<address16> earliest_dirty_addr_;
    uint32_t dirty_address_count_ = 0u;
    uint32_t earliest_dirty_bank_ = 0u;

    [[nodiscard]] std::pair<size_t, disassembly> disassemble(size_t physical_addr) noexcept;
    void generate_disassembly(size_t start) noexcept { generate_disassembly(start, data_.size()); }
    void generate_disassembly(size_t start, size_t end) noexcept;

    [[nodiscard]] uint16_t base_address() const noexcept
    {
        if(name_ == name_wram) { return 0xC000; }
        if(name_ == name_hram) { return 0xFF80; }
        return 0x0000;
    }
};

} // namespace instruction

} // namespace gameboy

#endif //GAMEBOY_DISASSEMBLY_DB_H
