#include <memory/controller/mbc3.h>
#include <memory/address_range.h>
#include <chrono>

uint8_t gameboy::rtc::read() const
{
    auto* time = std::localtime(&latched_time_);
    switch(selected_register_) {
        case register_type::seconds: return time->tm_sec;
        case register_type::minutes: return time->tm_min;
        case register_type::hours: return time->tm_hour;
        case register_type::days_lower_bits: return time->tm_yday & 0xFFu;
        case register_type::days_higher_bits: return (time->tm_yday & 0x100u) >> 8u;
        default: return 0u;
    }
}

void gameboy::rtc::write(const uint8_t data)
{
    selected_register_ = static_cast<register_type>(data - 0x80u);
}

void gameboy::rtc::latch()
{
    using namespace std::chrono;
    latched_time_ = system_clock::to_time_t(system_clock::now());
}

gameboy::mbc3::mbc3(const std::vector<uint8_t>& rom, const gameboy::cartridge& rom_header)
    :mbc(rom, rom_header) { }

uint8_t gameboy::mbc3::read(const gameboy::address16& virtual_address) const
{
    if(rtc_.is_enabled()) {
        return rtc_.read();
    }

    return mbc::read(virtual_address);
}

void gameboy::mbc3::control(const gameboy::address16& virtual_address, const uint8_t data)
{
    constexpr address_range external_ram_n_timer_enable_range(0x1FFFu);
    constexpr address_range rom_bank_select_range(0x2000u, 0x3FFFu);
    constexpr address_range ram_bank_or_rtc_reg_select_range(0x4000u, 0x5FFFu);
    constexpr address_range latch_clock_data_range(0x6000u, 0x7FFFu);

    if(external_ram_n_timer_enable_range.contains(virtual_address)) {
        set_external_ram_enabled(data);
    } else if(rom_bank_select_range.contains(virtual_address)) {
        select_rom_bank(data);
    } else if(ram_bank_or_rtc_reg_select_range.contains(virtual_address)) {
        if(data <= 0x03u) {
            select_ram_bank(data);
            rtc_.set_enabled(false);
        } else if(0x08u <= data && data <= 0x0Cu) {
            rtc_.set_enabled(true);
            rtc_.write(data);
        }
    } else if(latch_clock_data_range.contains(virtual_address)) {
        configure_latch(data);
    }
}

void gameboy::mbc3::select_rom_bank(const uint8_t data)
{
    rom_bank_ = data & 0x7Fu;
}

void gameboy::mbc3::select_ram_bank(const uint8_t data)
{
    ram_bank_ = data & 0x03u;
}

uint32_t gameboy::mbc3::get_rom_bank() const
{
    return rom_bank_ == 0u ? 0u : rom_bank_ - 1;
}

void gameboy::mbc3::configure_latch(const uint8_t data)
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
