#include <array>

#include "gameboy/timer/timer.h"
#include "gameboy/bus.h"
#include "gameboy/cpu/cpu.h"
#include "gameboy/memory/mmu.h"
#include "gameboy/util/delegate.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

constexpr address16 div_addr{0xFF04u};
constexpr address16 tima_addr{0xFF05u};
constexpr address16 tma_addr{0xFF06u};
constexpr address16 tac_addr{0xFF07u};

timer::timer(const observer<bus> bus)
    : bus_{bus},
      timer_clock_{0u},
      divider_clock_{0u},
      base_clock_{0u},
      div_{0x00u},
      tima_{0x00u},
      tma_{0x00u},
      tac_{0x00u}
{
    auto mmu = bus->get_mmu();

    for(const auto& addr : std::array{div_addr, tima_addr, tma_addr, tac_addr}) {
        mmu->add_memory_delegate({
            addr,
            {connect_arg<&timer::on_read>, this},
            {connect_arg<&timer::on_write>, this},
        });
    }
}

void timer::tick(const uint8_t cycles)
{
    // base clock dividers
    static constexpr std::array frequencies{
        64u, // 4   KHz
        1u,  // 262 KHz (base)
        4u,  // 65  KHz
        16u  // 16  KHz
    };

    timer_clock_ += cycles;

    while(timer_clock_ >= 16) {
        timer_clock_ -= 16;

        ++base_clock_;
        ++divider_clock_;

        if(divider_clock_ == 16) {
            div_ += 1;
            divider_clock_ = 0;
        }

        if(timer_enabled()) {
            const auto frequency = frequencies[timer_clock_select()];
            while(base_clock_ >= frequency) {
                base_clock_ -= frequency;

                if(div_ == 0xFFu) {
                    tima_ = tma_;
                    bus_->get_cpu()->request_interrupt(interrupt::timer);
                } else {
                    tima_ = tima_ + 1;
                }
            }
        }
    }
}

bool timer::timer_enabled() const noexcept
{
    return bit_test(tac_, 2);
}

std::size_t timer::timer_clock_select() const noexcept
{
    return tac_.value() & 0x03;
}

uint8_t timer::on_read(const address16& address) const noexcept
{
    if(address == div_addr) { return div_.value(); }
    if(address == tima_addr) { return tima_.value(); }
    if(address == tma_addr) { return tma_.value(); }
    if(address == tac_addr) { return tac_.value(); }

    return 0u;
}

void timer::on_write(const address16& address, const uint8_t data) noexcept
{
    if(address == div_addr) { div_ = 0u; }
    else if(address == tima_addr) { tima_ = data; }
    else if(address == tma_addr) { tma_ = data; }
    else if(address == tac_addr) { tac_ = data; }
}

} // namespace gameboy