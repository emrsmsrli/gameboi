#include "gameboy/timer/timer.h"

#include <array>

#include "gameboy/bus.h"
#include "gameboy/cpu/cpu.h"
#include "gameboy/memory/mmu.h"
#include "gameboy/util/delegate.h"

namespace gameboy {

constexpr address16 div_addr{0xFF04u};
constexpr address16 tima_addr{0xFF05u};
constexpr address16 tma_addr{0xFF06u};
constexpr address16 tac_addr{0xFF07u};

timer::timer(const observer<bus> bus)
    : bus_{bus},
      internal_clock_{0u},
      tima_reload_cycles_{0},
      tima_{0x00u},
      tma_{0x00u},
      tac_{0x00u},
      previous_tima_reload_bit_{false}
{
    auto mmu = bus->get_mmu();

    for(const auto& addr : std::array{div_addr, tima_addr, tma_addr, tac_addr}) {
        mmu->add_memory_delegate(addr, {
            {connect_arg<&timer::on_read>, this},
            {connect_arg<&timer::on_write>, this},
        });
    }
}

void timer::tick(const uint8_t cycles)
{
    update_internal_clock(internal_clock_ + cycles);
    if(tima_reload_cycles_ > 0) {
        tima_reload_cycles_ -= cycles;
        if(tima_reload_cycles_ <= 0) {
            tima_reload_cycles_ = 0;
            tima_ = tma_;
            bus_->get_cpu()->request_interrupt(interrupt::timer);
        }
    }
}

void timer::update_internal_clock(uint16_t new_internal_clock) noexcept
{
    internal_clock_ = new_internal_clock;

    const auto tima_reload_bit = bit::test(internal_clock_, timer_clock_overflow_index_select()) && timer_enabled();
    if(tima_reload_bit != previous_tima_reload_bit_) {
        tima_ += 1u;
        if(tima_ == 0x00u) {
            tima_reload_cycles_ = 24;
        }
    }

    previous_tima_reload_bit_ = tima_reload_bit;
}

uint8_t timer::timer_clock_overflow_index_select() const noexcept
{
    constexpr std::array<uint8_t, 4u> frequency_overflow_bit_indices{
        9u,  // 4   KHz
        3u,  // 256 KHz (base)
        5u,  // 64  KHz
        7u   // 16  KHz
    };

    const auto cycle_multiplier = bit::from_bool(bus_->get_cpu()->is_in_double_speed());
    return frequency_overflow_bit_indices[tac_.value() & 0x03u] << cycle_multiplier;
}

uint8_t timer::on_read(const address16& address) const noexcept
{
    if(address == div_addr) { return internal_clock_ >> 8u; }
    if(address == tima_addr) { return tima_.value(); }
    if(address == tma_addr) { return tma_.value(); }
    if(address == tac_addr) { return tac_.value(); }

    return 0u;
}

void timer::on_write(const address16& address, const uint8_t data) noexcept
{
    if(address == div_addr) { update_internal_clock(0u); }
    else if(address == tima_addr) {
        if(tima_reload_cycles_ < 4) {
            tima_reload_cycles_ = 0;
            tima_ = data;
        }
    }
    else if(address == tma_addr) { tma_ = data; }
    else if(address == tac_addr) { tac_ = data | 0xF8u; }
}

} // namespace gameboy
