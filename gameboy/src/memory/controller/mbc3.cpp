#include <chrono>

#include "gameboy/cartridge.h"
#include "gameboy/memory/address_range.h"
#include "gameboy/memory/controller/mbc3.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

std::time_t current_time() noexcept
{
    using namespace std::chrono;
    return system_clock::to_time_t(system_clock::now());
}

mbc3::mbc3(const observer<cartridge> cartridge, const std::pair<std::time_t, rtc>& rtc_data)
    : mbc(cartridge),
      rtc_{rtc_data.second},
      rtc_latch_{rtc_data.second},
      rtc_last_time_{rtc_data.first == 0u ? current_time() : rtc_data.first}
{
    update_rtc_latch();
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
    } else if(ram_bank_or_rtc_reg_select_range.has(address)) {
        if(data <= 0x03u) {
            ram_bank_ = data;
            rtc_enabled_ = false;
        } else if(0x08u <= data && data <= 0x0Cu) {
            if(cartridge_->has_rtc()) {
                rtc_enabled_ = true;
                rtc_selected_register_idx_ = data;
            }
        }
    } else if(latch_clock_data_range.has(address)) {
        if(cartridge_->has_rtc()) {
            if(rtc_latch_data_ == 0u && bit::test(data, 0)) {
                update_rtc_latch();
                rtc_latch_ = rtc_;
            }

            rtc_latch_data_ = data & 0x1u;
        }
    }
}

uint8_t mbc3::read_ram(const physical_address& address) const
{
    if(rtc_enabled_) {
        return rtc_latch_.read(rtc_selected_register_idx_);
    }

    return cartridge_->ram()[address.value()];
}

void mbc3::write_ram(const physical_address& address, const uint8_t data)
{
    if(rtc_enabled_) {
        switch(rtc_selected_register_idx_) {
            case 0x08u: rtc_.seconds = data;    break;
            case 0x09u: rtc_.minutes = data;    break;
            case 0x0Au: rtc_.hours = data;      break;
            case 0x0Bu: rtc_.days_lower = data; break;
            case 0x0Cu:
                rtc_.days_higher = (rtc_.days_higher & 0x80u) | (data & 0xC1u);
                break;
        }
    }

    cartridge_->ram()[address.value()] = data;
}

void mbc3::update_rtc_latch() noexcept
{
    const auto now = current_time();
    if(bit::test(rtc_.days_higher, 6u) || rtc_last_time_ == now) {
        return;
    }

    auto difference = now - rtc_last_time_;
    rtc_last_time_ = now;

    if(difference == 0) {
        return;
    }

    rtc_.seconds += difference % 60u;
    if(rtc_.seconds > 59u) {
        rtc_.seconds -= 60u;
        rtc_.minutes += 1u;
    }

    difference /= 60u;
    rtc_.minutes += difference % 60u;
    if(rtc_.minutes > 59u) {
        rtc_.minutes -= 60u;
        rtc_.hours += 1u;
    }

    auto rtc_days = static_cast<uint16_t>(rtc_.days_lower) + bit::extract(rtc_.days_higher, 0u);

    difference /= 60u;
    rtc_.hours += difference % 24u;
    if(rtc_.hours > 23u) {
        rtc_.hours -= 24u;
        rtc_days += 1u;
    }

    difference /= 24;
    rtc_days += difference;
    if(rtc_days > 511) {
        rtc_days %= 512;
        rtc_.days_higher |= 0x80u;
        rtc_.days_higher &= 0xC0u;
    }

    rtc_.days_lower = rtc_days & 0xFFu;
    rtc_.days_higher |= bit::extract(rtc_days, 8u);
}

} // namespace gameboy
