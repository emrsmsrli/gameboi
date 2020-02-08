#include "gameboy/link/link.h"
#include "gameboy/bus.h"
#include "gameboy/cpu/cpu.h"
#include "gameboy/memory/address.h"
#include "gameboy/memory/mmu.h"

namespace gameboy {

constexpr address16 sb_addr{0xFF01u};
constexpr address16 sc_addr{0xFF02u};

link::link(const observer<bus> bus) noexcept
    : bus_{bus}
{
    auto mmu = bus_->get_mmu();
    mmu->add_memory_delegate(sb_addr, {
        {connect_arg<&link::on_sb_read>, this},
        {connect_arg<&link::on_sb_write>, this}
    });
    mmu->add_memory_delegate(sc_addr, {
        {connect_arg<&link::on_sc_read>, this},
        {connect_arg<&link::on_sc_write>, this}
    });
}

void link::tick(const uint8_t cycles) noexcept
{
    if(!is_transferring()) {
        return;
    }

    if(clock_mode() == mode::internal) {
        shift_clock_ += cycles;
        if(const auto rate = clock_rate(); shift_clock_ >= rate) {
            shift_clock_ -= rate;

            shift_counter_++;
            if(shift_counter_ == 8u) {
                shift_counter_ = 0;

                sb_ = on_transfer_ ? on_transfer_(sb_.value()) : 0xFFu;
                sc_ = bit_reset(sc_, 7u);
            }
        }
    }
}

void link::on_sb_write(const address16&, const uint8_t data) noexcept
{
    sb_ = data;
}

uint8_t link::on_sb_read(const address16&) const noexcept
{
    return sb_.value();
}

void link::on_sc_write(const address16&, const uint8_t data) noexcept
{
    sc_ = data;
}

uint8_t link::on_sc_read(const address16&) const noexcept
{
    return sc_.value();
}

bool link::is_transferring() const noexcept
{
    return bit_test(sc_, 7u);
}

uint8_t link::clock_rate() const noexcept
{
    constexpr std::array clock_rates{16u, 512u};
    return bus_->get_cpu()->modified_cycles(clock_rates[extract_bit(sc_, 1u)]);
}

link::mode link::clock_mode() const noexcept
{
    return static_cast<mode>(extract_bit(sc_, 0u));
}

} // namespace gameboy
