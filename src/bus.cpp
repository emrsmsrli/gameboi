#include <bus.h>
#include <gameboy.h>

namespace gameboy {

observer<cartridge> bus::get_cartridge() const noexcept { return make_observer(gb_->cartridge_); }
observer<mmu> bus::get_mmu() const noexcept { return make_observer(gb_->mmu_); }
observer<cpu> bus::get_cpu() const noexcept { return make_observer(gb_->cpu_); }
observer<ppu> bus::get_ppu() const noexcept { return make_observer(gb_->ppu_); }
observer<apu> bus::get_apu() const noexcept { return make_observer(gb_->apu_); }

} // namespace gameboy
