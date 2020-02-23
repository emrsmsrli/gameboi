#ifndef GAMEBOY_CARTRIDGE_H
#define GAMEBOY_CARTRIDGE_H

#include <variant>
#include <vector>
#include <string>
#include <string_view>

#include "gameboy/memory/addressfwd.h"
#include "gameboy/memory/controller/mbc1.h"
#include "gameboy/memory/controller/mbc2.h"
#include "gameboy/memory/controller/mbc3.h"
#include "gameboy/memory/controller/mbc5.h"
#include "gameboy/memory/controller/mbc_regular.h"

namespace gameboy {
    
class cartridge_debugger;
class memory_bank_debugger;

class cartridge {
    friend cartridge_debugger;
    friend memory_bank_debugger;

public:
    explicit cartridge(std::string_view rom_path);

    [[nodiscard]] uint8_t read_rom(const address16& address) const;
    void write_rom(const address16& address, uint8_t data);

    [[nodiscard]] uint8_t read_ram(const address16& address) const;
    void write_ram(const address16& address, uint8_t data);

    [[nodiscard]] const std::vector<uint8_t>& rom() const noexcept { return rom_; }
    [[nodiscard]] const std::vector<uint8_t>& ram() const noexcept { return ram_; }

    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] bool cgb_enabled() const noexcept { return cgb_enabled_; }
    [[nodiscard]] bool has_battery() const noexcept { return has_battery_; }

private:
    bool cgb_enabled_ = false;
    bool has_battery_ = false;
    std::string_view cgb_type_;
    std::string_view mbc_type_;
    std::string_view rom_type_;
    std::string_view ram_type_;

    std::string name_;
    std::vector<uint8_t> rom_;
    std::vector<uint8_t> ram_;

    std::variant<mbc_regular, mbc1, mbc2, mbc3, mbc5> mbc_;

    [[nodiscard]] bool xram_enabled() const noexcept;
    [[nodiscard]] uint32_t rom_bank() const noexcept;
    [[nodiscard]] uint32_t ram_bank() const noexcept;

    [[nodiscard]] physical_address physical_ram_addr(const address16& address) const noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_CARTRIDGE_H
