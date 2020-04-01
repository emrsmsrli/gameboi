#ifndef GAMEBOY_MBC3_H
#define GAMEBOY_MBC3_H

#include <ctime>
#include <vector>

#include "gameboy/memory/controller/mbc.h"
#include "gameboy/memory/addressfwd.h"

namespace gameboy {

class cartridge_debugger;

class rtc {
    friend cartridge_debugger;

public:
    bool enabled = false;

    [[nodiscard]] uint8_t read() const noexcept;
    void write(uint8_t data) noexcept;
    void latch() noexcept;

private:
    enum class register_type : uint8_t {
        seconds = 0x08u,
        minutes = 0x09u,
        hours = 0x0Au,
        days_lower_bits = 0x0Bu,
        days_higher_bits = 0x0Cu
    };

    std::time_t latched_time_ = 0;
    register_type selected_register_{register_type::seconds};
};

class mbc3 : public mbc {
    friend cartridge_debugger;

public:
    explicit mbc3(const observer<cartridge> cartridge)
        : mbc(cartridge) {}

    void control(const address16& address, uint8_t data) noexcept;

    [[nodiscard]] uint8_t read_ram(const physical_address& address) const;
    void write_ram(const physical_address& address, uint8_t data) const;

private:
    rtc rtc_;
    bool rtc_latch_on_next_one_write_ = false;

    void configure_latch(uint8_t data);
};

} // namespace gameboy

#endif //GAMEBOY_MBC3_H
