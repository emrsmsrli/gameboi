#include <array>

#include <ppu/ppu.h>
#include <bus.h>
#include <cartridge.h>
#include <memory/address.h>
#include <util/mathutil.h>
#include <util/delegate.h>
#include <memory/memory_constants.h>

namespace gameboy {

constexpr auto hblank_cycles = 207;
constexpr auto reading_oam_cycles = 83;
constexpr auto reading_oam_vram_cycles = 175;

constexpr address16 lcdc_addr(0xFF40u);
constexpr address16 stat_addr(0xFF4u);

// Specifies the position in the 256x256 pixels BG map (32x32 tiles)
// which is to be displayed at the upper/left LCD display position.
// Values in range from 0-255 may be used for X/Y each, the video controller
// automatically wraps back to the upper (left) position in BG map when
// drawing exceeds the lower (right) border of the BG map area.
constexpr address16 scy_addr(0xFF42u);
constexpr address16 scx_addr(0xFF43u);

// The LY indicates the vertical line to which the present data is transferred
// to the LCD Driver. The LY can take on any value between 0 through 153.
// The values between 144 and 153 indicate the V-Blank period. Writing will reset the counter.
constexpr address16 ly_addr(0xFF44u);

// The gameboy permanently compares the value of the LYC and LY registers.
// When both values are identical, the coincident bit in the STAT register becomes set,
// and (if enabled) a STAT interrupt is requested.
constexpr address16 lyc_addr(0xFF45u);

// Specifies the upper/left positions of the Window area.
// (The window is an alternate background area which can be displayed above of the normal background.
// OBJs (sprites) may be still displayed above or behinf the window, just as for normal BG.)
// The window becomes visible (if enabled) when positions are set in range WX=0..166, WY=0..143.
// A postion of WX=7, WY=0 locates the window at upper left, it is then completly covering normal background.
constexpr address16 wy_addr(0xFF4Au);
constexpr address16 wx_addr(0xFF4Bu);

/*
 * This register assigns gray shades to the color numbers of the BG and Window tiles.
 *
 * Bit 7-6 - Shade for Color Number 3
 * Bit 5-4 - Shade for Color Number 2
 * Bit 3-2 - Shade for Color Number 1
 * Bit 1-0 - Shade for Color Number 0
 *
 * The four possible gray shades are:
 *
 * 0  White
 * 1  Light gray
 * 2  Dark gray
 * 3  Black
 *
 * In CGB Mode the Color Palettes are taken from CGB Palette Memory instead.
 */
constexpr address16 bgp_addr(0xFF47u);

// This register assigns gray shades for sprite palette 0.
// It works exactly as BGP (FF47), except that the lower
// two bits aren't used because sprite data 00 is transparent.
constexpr address16 obp_0_addr(0xFF48u);

//  This register assigns gray shades for sprite palette 1.
//  It works exactly as BGP (FF47), except that the lower
//  two bits aren't used because sprite data 00 is transparent.
constexpr address16 obp_1_addr(0xFF49u);

/*
 * This register is used to address a byte in the CGBs Background Palette Memory.
 * Each two byte in that memory define a color value.
 * The first 8 bytes define Color 0-3 of Palette 0 (BGP0), and so on for BGP1-7.
 *   Bit 0-5   Index (00-3F)
 *   Bit 7     Auto Increment  (0=Disabled, 1=Increment after Writing)
 *
 * Data can be read/written to/from the specified index address through Register FF69.
 * When the Auto Increment Bit is set then the index is automatically incremented after
 * each <write> to FF69. Auto Increment has no effect when <reading> from FF69,
 * so the index must be manually incremented in that case.
 */
constexpr address16 bgpi_addr(0xFF68u);

/*
 * This register allows to read/write data to the CGBs Background Palette Memory, addressed through Register FF68.
 * Each color is defined by two bytes (Bit 0-7 in first byte).
 *   Bit 0-4   Red Intensity   (00-1F)
 *   Bit 5-9   Green Intensity (00-1F)
 *   Bit 10-14 Blue Intensity  (00-1F)
 * Much like VRAM, Data in Palette Memory cannot be read/written during the
 * time when the LCD Controller is reading from it. (That is when the STAT register indicates Mode 3).
 * Note: Initially all background colors are initialized as white.
 */
constexpr address16 bgpd_addr(0xFF69u);

// These registers are used to initialize the Sprite Palettes OBP0-7,
// identically as described above for Background Palettes.
// Note that four colors may be defined for each OBP Palettes -
// but only Color 1-3 of each Sprite Palette can be displayed,
// Color 0 is always transparent, and can be initialized to a don't care value.
// Note: Initially all sprite colors are uninitialized.
constexpr address16 obpi_addr(0xFF6Au);
constexpr address16 obpd_addr(0xFF6Bu);

/** selects 1-bit vram bank register */
constexpr address16 vbk_addr(0xFF4Fu);

/*
 * Writing to this register launches a DMA transfer from ROM or RAM to OAM memory
 * (sprite attribute table). The written value specifies the transfer
 * source address divided by 100h, ie. source & destination are:
 *   Source:      XX00-XX9F   ;XX in range from 00-F1h
 *   Destination: FE00-FE9F
 *
 * It takes 160 microseconds until the transfer has completed
 * (80 microseconds in CGB Double Speed Mode), during this time
 * the CPU can access only HRAM (memory at FF80-FFFE). For this reason,
 * the programmer must copy a short procedure into HRAM, and use this
 * procedure to start the transfer from inside HRAM, and wait until the transfer has finished:
 *   ld  (0FF46h),a ;start DMA transfer, a=start address/100h
 *   ld  a,28h      ;delay...
 *   wait:          ;total 5x40 cycles, approx 200ms
 *   dec a          ;1 cycle
 *   jr  nz,wait    ;4 cycles
 * Most programs are executing this procedure from inside of their VBlank procedure,
 * but it is possible to execute it during display redraw also, allowing to display
 * more than 40 sprites on the screen (ie. for example 40 sprites in upper half,
 * and other 40 sprites in lower half of the screen).
 */
constexpr address16 oam_dma_addr(0xFF46u);

/*
 * These registers are used to initiate a DMA transfer from ROM or RAM to VRAM.
 * The Source Start Address may be located at 0000-7FF0 or A000-DFF0,
 * the lower four bits of the address are ignored (treated as zero).
 * The Destination Start Address may be located at 8000-9FF0, the lower four bits
 * of the address are ignored (treated as zero), the upper 3 bits are ignored either (destination is always in VRAM).
 *
 * Writing to FF55 starts the transfer, the lower 7 bits of FF55 specify the Transfer Length
 * (divided by 10h, minus 1). Ie. lengths of 10h-800h bytes can be defined by the values 00h-7Fh.
 * And the upper bit of FF55 indicates the Transfer Mode:
 *
 *   Bit7=0 - General Purpose DMA
 * When using this transfer method, all data is transferred at once.
 * The execution of the program is halted until the transfer has completed.
 * Note that the General Purpose DMA blindly attempts to copy the data,
 * even if the LCD controller is currently accessing VRAM.
 * So General Purpose DMA should be used only if the Display is disabled,
 * or during V-Blank, or (for rather short blocks) during H-Blank.
 * The execution of the program continues when the transfer has been completed, and FF55 then contains a value if FFh.
 *   Bit7=1 - H-Blank DMA
 * The H-Blank DMA transfers 10h bytes of data during each H-Blank,
 * ie. at LY=0-143, no data is transferred during V-Blank (LY=144-153),
 * but the transfer will then continue at LY=00. The execution of the program is
 * halted during the separate transfers, but the program execution continues
 * during the 'spaces' between each data block.
 * Note that the program may not change the Destination VRAM bank (FF4F),
 * or the Source ROM/RAM bank (in case data is transferred from bankable memory) until the transfer has completed!
 * Reading from Register FF55 returns the remaining length (divided by 10h, minus 1),
 * a value of 0FFh indicates that the transfer has completed. It is also possible to
 * terminate an active H-Blank transfer by writing zero to Bit 7 of FF55.
 * In that case reading from FF55 may return any value for the lower 7 bits,
 * but Bit 7 will be read as "1".
 *   Confirming if the DMA Transfer is Active
 * Reading Bit 7 of FF55 can be used to confirm if the DMA transfer is active (1=Not Active, 0=Active).
 * This works under any circumstances - after completion of General Purpose,
 * or H-Blank Transfer, and after manually terminating a H-Blank Transfer.
 *   Transfer Timings
 * In both Normal Speed and Double Speed Mode it takes about 8us to transfer a block of 10h bytes.
 * That are 8 cycles in Normal Speed Mode, and 16 'fast' cycles in Double Speed Mode.
 * Older MBC controllers (like MBC1-4) and slower ROMs are not guaranteed to support General Purpose or H-Blank DMA,
 * that's because there are always 2 bytes transferred per microsecond
 * (even if the itself program runs it Normal Speed Mode).
 */
constexpr address16 hdma_1_addr(0xFF51u); // New DMA Source, High
constexpr address16 hdma_2_addr(0xFF52u); // New DMA Source, Low
constexpr address16 hdma_3_addr(0xFF53u); // New DMA Destination, High
constexpr address16 hdma_4_addr(0xFF54u); // New DMA Destination, Low
constexpr address16 hdma_5_addr(0xFF55u); // New DMA Length/Mode/Start

ppu::ppu(observer<bus> bus)
    : bus_(bus),
      ram_((bus->get_cartridge()->cgb_enabled() ? 2 : 1) * 8_kb, 0u),
      oam_(oam_range.high() - oam_range.low() + 1, 0u)
{
    constexpr std::array dma_addresses{oam_dma_addr, hdma_1_addr, hdma_2_addr, hdma_3_addr, hdma_4_addr, hdma_5_addr};
    for(const auto& addr : dma_addresses) {
        bus->get_mmu()->add_memory_callback({
            addr,
            {connect_arg<&ppu::dma_read>, this},
            {connect_arg<&ppu::dma_write>, this}
        });
    }

    // todo register relevant above addresses to mmu
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
    const auto control = bus_->get_mmu()->read(lcdc_addr);
    return math::bit_test(control, flag);
}

uint8_t ppu::read(const address16& address) const
{
    return 0;
}

void ppu::write(const address16& address, const uint8_t data)
{

}

uint8_t ppu::dma_read(const address16& address) const
{
    if(address == hdma_1_addr) {
        return (dma_transfer_.source.value() & 0xFF00u) >> 8u;
    } else if(address == hdma_2_addr) {
        return dma_transfer_.source.value() & 0x00FFu;
    } else if(address == hdma_3_addr) {
        return (dma_transfer_.destination.value() & 0xFF00u) >> 8u;
    } else if(address == hdma_4_addr) {
        return dma_transfer_.destination.value() & 0x00FFu;
    } else if(address == hdma_5_addr) {
        return dma_transfer_.length_mode_start.value();
    }

    return 0u;
}

void ppu::dma_write(const address16& address, const uint8_t data)
{
    if(address == oam_dma_addr) {
        bus_->get_mmu()->dma(
            address16(data << 8u),
            make_address(oam_range.low()),
            oam_range.high() - oam_range.low());
    } else if(address == hdma_1_addr) {
        dma_transfer_.source = (dma_transfer_.source.value() & 0xFF00u) | (data & 0xF0u);
    } else if(address == hdma_2_addr) {
        dma_transfer_.source = (dma_transfer_.source.value() & 0x00FFu) | (data << 8u);
    } else if(address == hdma_3_addr) {
        dma_transfer_.destination = (dma_transfer_.destination.value() & 0xFF00u) | (data & 0xF0u);
    } else if(address == hdma_4_addr) {
        dma_transfer_.destination = (dma_transfer_.destination.value() & 0x00FFu) | ((data & 0x1Fu) << 8u);
    } else if(address == hdma_5_addr) {
        if(!math::bit_test(data, 7u)) {
            if(!dma_transfer_.active()) {
                bus_->get_mmu()->dma(
                    dma_transfer_.source,
                    dma_transfer_.destination,
                    dma_transfer_.length());
            } else {
                dma_transfer_.disable();
            }
        } else {
            dma_transfer_.remaining_length = dma_transfer_.length();
        }

        dma_transfer_.length_mode_start = data;
    }
}

void ppu::hdma()
{
    // todo Note that the program may not change the Destination VRAM bank (FF4F),
    // or the Source ROM/RAM bank (in case data is transferred from bankable memory)
    // until the transfer has completed!
    if(dma_transfer_.active() /* todo line >= 0 && line < 144 */) {
        const auto total_length = dma_transfer_.length();
        const auto offset = total_length - dma_transfer_.remaining_length;

        bus_->get_mmu()->dma(
            dma_transfer_.source + offset,
            dma_transfer_.destination + offset,
            0x10u);

        dma_transfer_.remaining_length -= 0x10u;
        if(dma_transfer_.remaining_length == 0u) {
            dma_transfer_.disable();
        }
    }
}

} // namespace gameboy
