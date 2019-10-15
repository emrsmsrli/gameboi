#include <ppu/ppu.h>
#include <memory/address.h>
#include <util/mathutil.h>

namespace gameboy {

constexpr auto hblank_cycles = 207;
constexpr auto reading_oam_cycles = 83;
constexpr auto reading_oam_vram_cycles = 175;

constexpr address16 addr_control(0xFF40u);
constexpr address16 addr_status(0xFF41u);

ppu::ppu(std::shared_ptr<mmu> memory_management_unit)
    : mmu_(std::move(memory_management_unit)),
      cycle_count_(0u)
{
    // todo register mmu callbacks for registers
}

void ppu::tick(const uint8_t cycles)
{
    if(!is_control_flag_set(control_flag::lcd_enable)) {
        return;
    }

    switch(mode_) {
        case mode::h_blank: {
            // if(hblank cycles elapsed) {
            // updateLy();
            // compareLyLyc();
            // if (line_ == VBLANK_LINE) {
            //     mode_ = Mode::VBLANK;
            //     set vblank interrupt provider
            //
            //     call on_vblank()
            // } else {
            //     mode_ = Mode::reading_oam;
            // }
            //
            // checkStatInterrupts(ime);
            // }
            break;
        }
        case mode::v_blank: {
            // if (hasElapsed(LINE_CYCLES)) {
            //     updateLY();
            //     compareLyToLyc(ime);
            //     if (line_ == 0) {
            //         mode_ = Mode::reading_oam;
            //         checkStatInterrupts(ime);
            //     }
            // }
            break;
        }
        case mode::reading_oam: {
            // if (hasElapsed(OAM_ACCESS_CYCLES)) {
            //     mode_ = Mode::reading_oam_vram;
            // }
            break;
        }
        case mode::reading_oam_vram: {
            // if (hasElapsed(LCD_TRANSFER_CYCLES)) {
            //     renderScanline();
            //     mode_ = Mode::HBLANK;
            //     doHdma();
            //     checkStatInterrupts(ime);
            // }
            break;
        }
    }
}

bool ppu::is_control_flag_set(const ppu::control_flag flag) const
{
    const auto control = mmu_->read(addr_control);
    return math::bit_test(control, flag);
}

} // namespace gameboy
