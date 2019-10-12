#ifndef GAMEBOY_CARTRIDGE_H
#define GAMEBOY_CARTRIDGE_H

#include <variant>
#include <vector>
#include <string>
#include <cstddef>

#include <memory/addressfwd.h>

namespace gameboy {

class mbc1;
class mbc2;
class mbc3;
class mbc5;
class mbc_null;

class cartridge {
    friend class rom_parser;

public:
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] bool cgb_enabled() const noexcept { return cgb_enabled_; }

    [[nodiscard]] uint8_t read_rom(const address16& address) const;
    void write_rom(const address16& address, uint8_t data);

    [[nodiscard]] uint8_t read_ram(const address16& address) const;
    void write_ram(const address16& address, uint8_t data);

private:
    bool cgb_enabled_ = false;

    std::string name_;
    std::vector<uint8_t> rom_;
    std::vector<uint8_t> ram_;

    std::variant<mbc_null, mbc1, mbc2, mbc3, mbc5> mbc_;
};

}

#endif //GAMEBOY_CARTRIDGE_H
