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
#include "gameboy/util/fileutil.h"

namespace gameboy {
    
class cartridge_debugger;
class memory_bank_debugger;

class cartridge {
    friend cartridge_debugger;
    friend memory_bank_debugger;

public:
    explicit cartridge(const filesystem::path& rom_path);
    ~cartridge();

    [[nodiscard]] uint8_t read_rom(const address16& address) const;
    void write_rom(const address16& address, uint8_t data);

    [[nodiscard]] uint8_t read_ram(const address16& address) const;
    void write_ram(const address16& address, uint8_t data);

    [[nodiscard]] std::vector<uint8_t>& rom() noexcept { return rom_; }
    [[nodiscard]] const std::vector<uint8_t>& rom() const noexcept { return rom_; }
    [[nodiscard]] uint32_t rom_bank_count() const noexcept { return rom_bank_count_; }

    [[nodiscard]] std::vector<uint8_t>& ram() noexcept { return ram_; }
    [[nodiscard]] const std::vector<uint8_t>& ram() const noexcept { return ram_; }
    [[nodiscard]] uint32_t ram_bank_count() const noexcept { return ram_bank_count_; }

    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] bool cgb_enabled() const noexcept { return cgb_enabled_; }

    [[nodiscard]] bool has_battery() const noexcept { return has_battery_; }
    [[nodiscard]] bool has_rtc() const noexcept { return has_rtc_; }

private:
    filesystem::path rom_path_;

    bool cgb_enabled_ = false;
    bool has_battery_ = false;
    bool has_rtc_ = false;

    uint32_t rom_bank_count_ = 0u;
    uint32_t ram_bank_count_ = 0u;

    std::string_view cgb_type_;
    std::string_view mbc_type_;
    std::string_view rom_type_;
    std::string_view ram_type_;

    std::string name_;
    std::vector<uint8_t> rom_;
    std::vector<uint8_t> ram_;

    std::variant<mbc_regular, mbc1, mbc2, mbc3, mbc5> mbc_;

    [[nodiscard]] bool ram_enabled() const noexcept;
    [[nodiscard]] uint32_t rom_bank() const noexcept;
    [[nodiscard]] uint32_t ram_bank() const noexcept;

    [[nodiscard]] physical_address physical_ram_addr(const address16& address) const noexcept;

    void load_ram();
    void save_ram();
};

} // namespace gameboy

#endif //GAMEBOY_CARTRIDGE_H
