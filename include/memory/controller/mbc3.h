#ifndef GAMEBOY_MBC3_H
#define GAMEBOY_MBC3_H

#include <ctime>

#include <memory/controller/mbc.h>
#include <memory/addressfwd.h>

namespace gameboy {

class rtc {
public:
    bool enabled = false;

    [[nodiscard]] uint8_t read() const noexcept;
    void write(uint8_t data) noexcept;
    void latch() noexcept;

private:
    enum class register_type {
        seconds,
        minutes,
        hours,
        days_lower_bits,
        days_higher_bits
    };

    std::time_t latched_time_ = 0;
    register_type selected_register_{register_type::seconds};
};

class mbc3 : public mbc {
public:
    void control(const address16& address, uint8_t data) noexcept;

    [[nodiscard]] uint8_t read_ram(const std::vector<uint8_t>& ram, const physical_address& address) const;
    void write_ram(std::vector<uint8_t>& ram, const physical_address& address, uint8_t data) const;

private:
    rtc rtc_;
    bool rtc_latch_on_next_one_write_ = false;

    void configure_latch(uint8_t data);
};

} // namespace gameboy

#endif //GAMEBOY_MBC3_H
