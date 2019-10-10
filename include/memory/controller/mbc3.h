#ifndef GAMEBOY_MBC3_H
#define GAMEBOY_MBC3_H

#include <memory/controller/mbc.h>
#include <ctime>

namespace gameboy {

class rtc {
public:
    [[nodiscard]] uint8_t read() const;
    void write(uint8_t data);
    void latch();

    [[nodiscard]] bool is_enabled() const { return enabled_; }
    void set_enabled(const bool enable) { enabled_ = enable; }

private:
    enum class register_type {
        seconds,
        minutes,
        hours,
        days_lower_bits,
        days_higher_bits
    };

    bool enabled_ = false;
    std::time_t latched_time_ = 0;
    register_type selected_register_{register_type::seconds};
};

class mbc3 : public mbc {
public:
    mbc3(const std::vector<uint8_t>& rom, const cartridge& rom_header);

    [[nodiscard]] uint8_t read(const address16& virtual_address) const override;

protected:
    void select_rom_bank(uint8_t data) override;
    void select_ram_bank(uint8_t data) override;

private:
    rtc rtc_;
    bool rtc_latch_on_next_one_write_ = false;

    void configure_latch(uint8_t data);

    [[nodiscard]] uint32_t get_rom_bank() const override;
    void control(const address16& virtual_address, uint8_t data) override;
};

}

#endif //GAMEBOY_MBC3_H
