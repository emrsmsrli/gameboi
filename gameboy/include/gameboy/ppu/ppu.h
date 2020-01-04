#ifndef GAMEBOY_PPU_H
#define GAMEBOY_PPU_H

#include <vector>

#include "gameboy/ppu/dma_transfer_data.h"
#include "gameboy/ppu/color.h"
#include "gameboy/ppu/register_stat.h"
#include "gameboy/ppu/register_lcdc.h"
#include "gameboy/ppu/data/obj.h"
#include "gameboy/ppu/data/palette.h"
#include "gameboy/memory/addressfwd.h"
#include "gameboy/util/delegate.h"
#include "gameboy/util/observer.h"

namespace gameboy {

class bus;
class ppu_debugger;
class memory_bank_debugger;

static constexpr auto screen_width = 160;
static constexpr auto screen_height = 144;

using render_line = std::array<color, screen_width>;

class ppu {
    friend ppu_debugger;
    friend memory_bank_debugger;

public:
    // The LY indicates the vertical line to which the present data is transferred
    // to the LCD Driver. The LY can take on any value between 0 through 153.
    // The values between 144 and 153 indicate the V-Blank period. Writing will reset the counter.
    static constexpr address16 ly_addr{0xFF44u};

    delegate<void(uint8_t, const render_line&)> on_render_line;
    delegate<void()> on_render_frame;

    explicit ppu(observer<bus> bus);

    void tick(uint8_t cycles);

    [[nodiscard]] uint8_t read(const address16& address) const;
    void write(const address16& address, uint8_t data);

private:
    observer<bus> bus_;

    uint8_t cycle_count_;
    uint8_t vram_bank_;

    std::vector<uint8_t> ram_;
    std::vector<uint8_t> oam_;

    register_lcdc lcdc_;
    register_stat stat_;

    register8 ly_;
    register8 lyc_;

    register8 scx_;
    register8 scy_;
    register8 wx_;
    register8 wy_;

    static constexpr palette gb_palette_{
        color{255u},
        color{192u},
        color{96u},
        color{0u}
    };
    register8 bgp_;
    std::array<register8, 2> obp_;

    std::array<palette, 8> cgb_bg_palettes_;
    std::array<palette, 8> cgb_obj_palettes_;
    register8 bgpi_;
    register8 obpi_;

    dma_transfer_data dma_transfer_;

    [[nodiscard]] uint8_t dma_read(const address16& address) const;
    void dma_write(const address16& address, uint8_t data);

    [[nodiscard]] uint8_t general_purpose_register_read(const address16& address) const;
    void general_purpose_register_write(const address16& address, uint8_t data);

    [[nodiscard]] uint8_t palette_read(const address16& address) const;
    void palette_write(const address16& address, uint8_t data);

    void hdma();
    void render() const noexcept;

    void render_background() const noexcept;
    void render_window() const noexcept;
    void render_obj() const noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_PPU_H
