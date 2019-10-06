#ifndef GAMEBOY_MBC3_H
#define GAMEBOY_MBC3_H

#include <memory/controller/MBC.h>
#include <ctime>

namespace gameboy::memory::controller {
    class RTC {
    public:
        [[nodiscard]] uint8_t read() const;
        void write(uint8_t data);
        void latch();

        [[nodiscard]] bool is_enabled() const { return enabled; }
        void set_enabled(const bool enable) { enabled = enable; }

    private:
        enum class RegisterType {
            seconds,
            minutes,
            hours,
            days_lower_bits,
            days_higher_bits
        };

        bool enabled = false;
        std::time_t latched_time = 0;
        RegisterType selected_register{RegisterType::seconds};
    };

    class MBC3 : public MBC {
    public:
        MBC3(const std::vector<uint8_t>& rom, const CartridgeInfo& rom_header);

        [[nodiscard]] uint8_t read(const Address16& virtual_address) const override;

    protected:
        void select_rom_bank(uint8_t data) override;
        void select_ram_bank(uint8_t data) override;

    private:
        RTC rtc;
        bool rtc_latch_on_next_one_write = false;

        void configure_latch(uint8_t data);

        [[nodiscard]] uint32_t get_rom_bank() const override;
        void control(const Address16& virtual_address, uint8_t data) override;
    };
}

#endif //GAMEBOY_MBC3_H
