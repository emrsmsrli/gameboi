#ifndef GAMEBOY_CARTRIDGE_H
#define GAMEBOY_CARTRIDGE_H

#include <variant>
#include <vector>
#include <string>
#include <string_view>
#include <cstddef>

#include <memory/addressfwd.h>
#include <memory/controller/mbc1.h>
#include <memory/controller/mbc2.h>
#include <memory/controller/mbc3.h>
#include <memory/controller/mbc_regular.h>

namespace gameboy {

class cartridge {
public:
    explicit cartridge(std::string_view rom);

    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] bool cgb_enabled() const noexcept { return cgb_enabled_; }

    [[nodiscard]] uint8_t read_rom(const address16& address) const;
    void write_rom(const address16& address, uint8_t data);

    [[nodiscard]] uint8_t read_ram(const address16& address) const;
    void write_ram(const address16& address, uint8_t data);

    [[nodiscard]] const std::vector<uint8_t>& ram() const noexcept { return ram_; }

private:
    bool cgb_enabled_ = false;

    std::string name_;
    std::vector<uint8_t> rom_;
    std::vector<uint8_t> ram_;

    std::variant<mbc_regular, mbc1, mbc2, mbc3> mbc_;

    [[nodiscard]] bool xram_enabled() const noexcept;
    [[nodiscard]] uint32_t rom_bank() const noexcept;
    [[nodiscard]] uint32_t ram_bank() const noexcept;

    [[nodiscard]] physical_address physical_ram_addr(const address16& address) const noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_CARTRIDGE_H
