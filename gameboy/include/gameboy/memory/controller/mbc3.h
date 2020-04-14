#ifndef GAMEBOY_MBC3_H
#define GAMEBOY_MBC3_H

#include <ctime>
#include <vector>
#include <utility> // std::pair

#include "gameboy/memory/controller/mbc.h"
#include "gameboy/memory/addressfwd.h"

namespace gameboy {

class cartridge_debugger;

struct rtc {
    uint8_t seconds = 0u;
    uint8_t minutes = 0u;
    uint8_t hours = 0u;
    uint8_t days_lower = 0u;
    uint8_t days_higher = 0u;

    [[nodiscard]] uint8_t read(const uint8_t idx) const noexcept
    {
        switch(idx) {
            case 0x08u: return seconds;
            case 0x09u: return minutes;
            case 0x0Au: return hours;
            case 0x0Bu: return days_lower;
            case 0x0Cu: return days_higher;
            default:    return 0xFFu;
        }
    }
};

class mbc3 : public mbc {
    friend cartridge_debugger;

public:
    mbc3(observer<cartridge> cartridge, const std::pair<std::time_t, rtc>& rtc_data);

    void control(const address16& address, uint8_t data) noexcept;

    [[nodiscard]] uint8_t read_ram(const physical_address& address) const;
    void write_ram(const physical_address& address, uint8_t data);

    [[nodiscard]] std::pair<std::time_t, rtc> get_rtc_data() const noexcept { return std::make_pair(rtc_last_time_, rtc_); }

private:
    rtc rtc_;
    rtc rtc_latch_;
    std::time_t rtc_last_time_ = 0u;
    bool rtc_enabled_ = false;
    uint8_t rtc_latch_data_ = 0u;
    uint8_t rtc_selected_register_idx_ = 0x00u;

    void update_rtc_latch() noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_MBC3_H
