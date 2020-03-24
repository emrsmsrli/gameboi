#include <chrono>

#include "gameboy/memory/controller/mbc3.h"
#include "gameboy/memory/address_range.h"
#include "gameboy/cartridge.h"

namespace gameboy {

uint8_t rtc::read() const noexcept
{
    auto* time = std::localtime(&latched_time_);

    switch(selected_register_) {
        case register_type::seconds: return time->tm_sec;
        case register_type::minutes: return time->tm_min;
        case register_type::hours: return time->tm_hour;
        case register_type::days_lower_bits: return time->tm_yday & 0xFFu;
        case register_type::days_higher_bits: return (time->tm_yday & 0x100u) >> 8u;
        default: return 0xFFu;
    }
}

void rtc::write(const uint8_t data) noexcept
{
    selected_register_ = static_cast<register_type>(data);
}

void rtc::latch() noexcept
{
    using namespace std::chrono;
    latched_time_ = system_clock::to_time_t(system_clock::now());
}

void mbc3::control(const address16& address, const uint8_t data) noexcept
{
    constexpr address_range external_ram_n_timer_enable_range{0x1FFFu};
    constexpr address_range rom_bank_select_range{0x2000u, 0x3FFFu};
    constexpr address_range ram_bank_or_rtc_reg_select_range{0x4000u, 0x5FFFu};
    constexpr address_range latch_clock_data_range{0x6000u, 0x7FFFu};

    if(external_ram_n_timer_enable_range.has(address)) {
        if(cartridge_->ram_bank_count() != 0u) {
            set_ram_enabled(data);
        }
    } else if(rom_bank_select_range.has(address)) {
        rom_bank_ = data & 0x7Fu;
        if(rom_bank_ == 0u) {
            rom_bank_ = 1u;
        }

        rom_bank_ &= cartridge_->rom_bank_count() - 1u;
    } else if(ram_bank_or_rtc_reg_select_range.has(address)) {
        if(data <= 0x03u) {
            ram_bank_ = data;
            ram_bank_ &= cartridge_->ram_bank_count() - 1u;
            rtc_.enabled = false;
        } else if(0x08u <= data && data <= 0x0Cu) {
            if(cartridge_->has_rtc()) {
                rtc_.enabled = true;
                rtc_.write(data);
            }
        }
    } else if(latch_clock_data_range.has(address)) {
        if(cartridge_->has_rtc()) {
            configure_latch(data);
        }
    }
}

uint8_t mbc3::read_ram(const physical_address& address) const
{
    if(rtc_.enabled) {
        return rtc_.read();
    }

    return cartridge_->ram()[address.value()];
}

void mbc3::write_ram(const physical_address& address, const uint8_t data) const
{
    cartridge_->ram()[address.value()] = data;
}

void mbc3::configure_latch(const uint8_t data)
{
    // consecutive 0x0, 0x1 writes causes rtc to latch current time
    if(data == 0x00u) {
        rtc_latch_on_next_one_write_ = true;
    } else if(rtc_latch_on_next_one_write_ && data == 0x01u) {
        rtc_latch_on_next_one_write_ = false;
        rtc_.latch();
    } else {
        rtc_latch_on_next_one_write_ = false;
    }
}

} // namespace gameboy
