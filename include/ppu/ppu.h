#ifndef GAMEBOY_PPU_H
#define GAMEBOY_PPU_H

#include <vector>
#include <memory>

#include <ppu/dma_transfer_data.h>
#include <ppu/color.h>
#include <ppu/register_stat.h>
#include <ppu/register_lcdc.h>
#include <ppu/data/palette.h>
#include <memory/addressfwd.h>
#include <util/delegate.h>
#include <util/observer.h>
#include <util/mathutil.h>

namespace gameboy {

class bus;

static constexpr auto screen_width = 160;
static constexpr auto screen_height = 144;

using render_line = std::array<color, screen_width>;

class ppu {
public:
    delegate<void(uint8_t, const render_line&)> on_render_line;

    explicit ppu(observer<bus> bus);

    void tick(uint8_t cycles);

    [[nodiscard]] uint8_t read(const address16& address) const;
    void write(const address16& address, uint8_t data);

private:
    observer<bus> bus_;

    std::vector<uint8_t> ram_;
    std::vector<uint8_t> oam_;

    uint8_t cycle_count_ = 0;

    register_lcdc lcdc_;
    register_stat stat_;

    register8 ly_;
    register8 lyc_;

    register8 scx_;
    register8 scy_;
    register8 wx_;
    register8 wy_;

    static constexpr palette gb_palette_{};
    register8 bgp_;
    std::array<register8, 2> obp_;

    std::array<palette, 8> cgb_bg_palettes_;
    std::array<palette, 8> cgb_obj_palettes_;
    register8 bgpi_;
    register8 obpi_;

    uint8_t vram_bank_ = 0u;

    dma_transfer_data dma_transfer_;

    [[nodiscard]] uint8_t dma_read(const address16& address) const;
    void dma_write(const address16& address, uint8_t data);

    [[nodiscard]] uint8_t general_purpose_register_read(const address16& address) const;
    void general_purpose_register_write(const address16& address, uint8_t data);

    [[nodiscard]] uint8_t palette_read(const address16& address) const;
    void palette_write(const address16& address, uint8_t data);

    void hdma();
    void render();
};

} // namespace gameboy

#endif //GAMEBOY_PPU_H
