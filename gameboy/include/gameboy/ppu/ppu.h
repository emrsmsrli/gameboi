#ifndef GAMEBOY_PPU_H
#define GAMEBOY_PPU_H

#include <array>
#include <vector>
#include <variant>
#include <initializer_list>

#include "gameboy/ppu/dma_transfer_data.h"
#include "gameboy/ppu/color.h"
#include "gameboy/ppu/register_stat.h"
#include "gameboy/ppu/register_lcdc.h"
#include "gameboy/ppu/data/interrupt_request.h"
#include "gameboy/ppu/data/attributes.h"
#include "gameboy/ppu/data/palette.h"
#include "gameboy/memory/addressfwd.h"
#include "gameboy/util/delegate.h"
#include "gameboy/util/observer.h"

namespace gameboy {

class bus;
class ppu_debugger;
class memory_bank_debugger;

static constexpr auto screen_width = 160u;
static constexpr auto screen_height = 144u;

using render_line = std::array<color, screen_width>;

class ppu {
    friend ppu_debugger;
    friend memory_bank_debugger;

public:
    using render_line_func = delegate<void(uint8_t, const render_line&)>;
    using vblank_func = delegate<void()>;

    static constexpr address16 ly_addr{0xFF44u};
    static constexpr palette palette_grayscale{
        color{255u},
        color{192u},
        color{96u},
        color{0u}
    };
    static constexpr palette palette_zelda{
        color{224, 248, 208},
        color{136, 192, 112},
        color{52, 104, 84},
        color{8, 24, 32}
    };
    static constexpr palette palette_gold{
        color{252, 232, 140},
        color{220, 180, 92},
        color{152, 124, 60},
        color{76,60, 28}
    };
    static constexpr palette palette_green{
        color{155, 188, 15},
        color{139, 172, 15},
        color{48, 98, 48},
        color{15, 56, 15}
    };

    explicit ppu(observer<bus> bus);

    void tick(uint8_t cycles);
    void on_render_line(const render_line_func on_render_line) noexcept { on_render_line_ = on_render_line; }
    void on_vblank(const vblank_func on_vblank) noexcept { on_vblank_ = on_vblank; }

    void set_gb_palette(const palette& palette) noexcept { gb_palette_ = palette; }

    [[nodiscard]] uint8_t read_ram(const address16& address) const;
    void write_ram(const address16& address, uint8_t data);

    [[nodiscard]] uint8_t read_oam(const address16& address) const;
    void write_oam(const address16& address, uint8_t data);

private:
    using tile_attribute = std::variant<attributes::uninitialized, attributes::bg, attributes::obj>;
    using render_buffer = std::array<std::pair<uint8_t, tile_attribute>, screen_width>;

    static constexpr auto map_tile_count = 32u;
    static constexpr auto tile_pixel_count = 8u;
    static constexpr auto map_pixel_count = map_tile_count * tile_pixel_count;

    observer<bus> bus_;

    bool lcd_enabled_;
    bool line_rendered_;
    int8_t vblank_line_;
    int8_t lcd_enable_delay_frame_count_;
    int16_t lcd_enable_delay_cycle_count_;
    uint32_t cycle_count_;
    uint32_t secondary_cycle_count_;
    uint8_t vram_bank_;

    std::vector<uint8_t> ram_;
    std::vector<uint8_t> oam_;

    interrupt_request interrupt_request_;
    register_lcdc lcdc_;
    register_stat stat_;

    register8 ly_;
    register8 lyc_;

    register8 scx_;
    register8 scy_;
    register8 wx_;
    register8 wy_;

    palette gb_palette_;
    register8 bgp_;
    std::array<register8, 2> obp_;

    std::array<palette, 8> cgb_bg_palettes_;
    std::array<palette, 8> cgb_obj_palettes_;
    register8 bgpi_;
    register8 obpi_;

    dma_transfer_data dma_transfer_;

    render_line_func on_render_line_;
    vblank_func on_vblank_;

    [[nodiscard]] uint8_t read_ram_by_bank(const address16& address, uint8_t bank) const;
    void write_ram_by_bank(const address16& address, uint8_t data, uint8_t bank);

    [[nodiscard]] uint8_t dma_read(const address16& address) const;
    void dma_write(const address16& address, uint8_t data);

    [[nodiscard]] uint8_t general_purpose_register_read(const address16& address) const;
    void general_purpose_register_write(const address16& address, uint8_t data);

    [[nodiscard]] uint8_t palette_read(const address16& address) const;
    void palette_write(const address16& address, uint8_t data);

    void compare_coincidence() noexcept;
    void set_ly(const register8& ly) noexcept;
    void set_lyc(const register8& lyc) noexcept;

    void disable_screen() noexcept;

    void request_interrupt(interrupt_request::type type) noexcept;
    void request_interrupt(interrupt_request& irq, interrupt_request::type type) noexcept;
    void reset_interrupt_requests(std::initializer_list<interrupt_request::type> irqs) noexcept;

    void hdma();
    void gdma();
    void render() const noexcept;

    void render_background(render_buffer& buffer) const noexcept;
    void render_window(render_buffer& buffer) const noexcept;
    void render_obj(render_buffer& buffer) const noexcept;

    [[nodiscard]] std::array<uint8_t, tile_pixel_count> get_tile_row(
        uint8_t row, uint8_t tile_no, uint8_t bank) const noexcept;
    [[nodiscard]] std::array<uint8_t, tile_pixel_count> get_tile_row(
        uint8_t row, const address16& tile_base_addr, uint8_t bank) const noexcept;

    [[nodiscard]] static color correct_color(const color& c) noexcept;

    template<typename T>
    [[nodiscard]] static address16 tile_address(const uint32_t base_addr, const uint8_t tile_no)
    {
        return address16{static_cast<uint16_t>(base_addr + static_cast<T>(tile_no) * tile_pixel_count * 2)};
    }
};

} // namespace gameboy

#endif //GAMEBOY_PPU_H
