#include <memory/controller/MBC3.h>
#include <memory/AddressRange.h>
#include <chrono>

uint8_t gameboy::memory::controller::RTC::read() const
{
    std::tm* time = std::localtime(&latched_time);
    switch(selected_register) {
        case RegisterType::seconds:
            return time->tm_sec;
        case RegisterType::minutes:
            return time->tm_min;
        case RegisterType::hours:
            return time->tm_hour;
        case RegisterType::days_lower_bits:
            return time->tm_yday & 0xFFu;
        case RegisterType::days_higher_bits:
            return (time->tm_yday & 0x100u) >> 8u;
    }
}

void gameboy::memory::controller::RTC::write(uint8_t data)
{
    selected_register = static_cast<RegisterType>(data - 0x80u);
}

void gameboy::memory::controller::RTC::latch()
{
    using namespace std::chrono;
    latched_time = system_clock::to_time_t(system_clock::now());
}

gameboy::memory::controller::MBC3::MBC3(const std::vector<uint8_t>& rom, const gameboy::CartridgeInfo& rom_header)
        : MBC(rom, rom_header) { }

uint8_t gameboy::memory::controller::MBC3::read(const gameboy::memory::Address16& virtual_address) const
{
    if(rtc.is_enabled()) {
        return rtc.read();
    }

    return MBC::read(virtual_address);
}

void gameboy::memory::controller::MBC3::control(const gameboy::memory::Address16& virtual_address, uint8_t data)
{
    constexpr AddressRange external_ram_n_timer_enable_range(0x1FFFu);
    constexpr AddressRange rom_bank_select_range(0x2000u, 0x3FFFu);
    constexpr AddressRange ram_bank_or_rtc_reg_select_range(0x4000u, 0x5FFFu);
    constexpr AddressRange latch_clock_data_range(0x6000u, 0x7FFFu);

    if(external_ram_n_timer_enable_range.contains(virtual_address)) {
        set_external_ram_enabled(data);
    } else if(rom_bank_select_range.contains(virtual_address)) {
        select_rom_bank(data);
    } else if(ram_bank_or_rtc_reg_select_range.contains(virtual_address)) {
        if(data <= 0x03u) {
            select_ram_bank(data);
            rtc.set_enabled(false);
        } else if(0x08u <= data && data <= 0x0Cu) {
            rtc.set_enabled(true);
            rtc.write(data);
        }
    } else if(latch_clock_data_range.contains(virtual_address)) {
        configure_latch(data);
    }
}

void gameboy::memory::controller::MBC3::select_rom_bank(uint8_t data)
{
    rom_bank = data & 0x7Fu;
}

void gameboy::memory::controller::MBC3::select_ram_bank(uint8_t data)
{
    ram_bank = data & 0x03u;
}

uint32_t gameboy::memory::controller::MBC3::get_rom_bank() const
{
    return rom_bank == 0u ? 0u : rom_bank - 1;
}

void gameboy::memory::controller::MBC3::configure_latch(uint8_t data)
{
    // consecutive 0x0, 0x1 writes causes rtc to latch current time
    if(data == 0x00u) {
        rtc_latch_on_next_one_write = true;
    } else if(rtc_latch_on_next_one_write && data == 0x01u) {
        rtc_latch_on_next_one_write = false;
        rtc.latch();
    } else {
        rtc_latch_on_next_one_write = false;
    }
}
