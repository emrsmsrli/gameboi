#include "gameboy/link/link.h"
#include "gameboy/bus.h"
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
    
}

void link::on_sb_write(const address16&, uint8_t data) noexcept
{
    
}

uint8_t link::on_sb_read(const address16&) const noexcept
{
    return 0u;
}

void link::on_sc_write(const address16&, uint8_t data) noexcept
{
    
}

uint8_t link::on_sc_read(const address16&) const noexcept
{
    return 0u;
}

} // namespace gameboy
