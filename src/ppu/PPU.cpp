#include <ppu/PPU.h>
#include <memory/Address.h>
#include <util/Math.h>

namespace {
    constexpr auto hblank_cycles = 207;
    constexpr auto reading_oam_cycles = 83;
    constexpr auto reading_oam_vram_cycles = 175;

    constexpr gameboy::memory::Address16 addr_control(0xFF40u);
    constexpr gameboy::memory::Address16 addr_status(0xFF41u);
}

gameboy::ppu::PPU::PPU(std::shared_ptr<memory::MMU> memory_management_unit)
        : mmu(std::move(memory_management_unit)),
          cycle_count(0u)
{
    // todo register mmu callbacks for registers
}

void gameboy::ppu::PPU::tick(const uint8_t cycles)
{
    if(!is_control_flag_set(ControlFlag::lcd_enable)) {
        return;
    }

    switch(mode) {
        case Mode::h_blank: {
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
        case Mode::v_blank: {
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
        case Mode::reading_oam: {
            // if (hasElapsed(OAM_ACCESS_CYCLES)) {
            //     mode_ = Mode::reading_oam_vram;
            // }
            break;
        }
        case Mode::reading_oam_vram: {
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

bool gameboy::ppu::PPU::is_control_flag_set(const gameboy::ppu::ControlFlag flag) const
{
    const auto control = mmu->read(addr_control);
    return math::bit_test(control, flag);
}
