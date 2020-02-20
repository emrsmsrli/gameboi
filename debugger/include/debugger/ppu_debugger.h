#ifndef GAMEBOY_PPU_DEBUGGER_H
#define GAMEBOY_PPU_DEBUGGER_H

#include <array>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "gameboy/util/observer.h"

namespace gameboy {

class ppu;

class ppu_debugger {
public:
    explicit ppu_debugger(observer<ppu> ppu) noexcept;
   
    void draw() noexcept;

private:
    observer<ppu> ppu_;
    sf::Image tiles_img_;
    sf::Texture tiles_;

    std::array<sf::Image, 32u * 32u> bg_map_imgs_; 
    std::array<sf::Texture, 32u * 32u> bg_map_; 

    std::array<sf::Image, 40u> oam_imgs_; 
    std::array<sf::Texture, 40u> oam_; 

    void draw_registers() const noexcept;
    void draw_lcdc_n_stat() const;
    void draw_palettes() const;
    void draw_dma() const noexcept;

    void draw_vram_view();
    void draw_tiles();
    void draw_bg_map();
    void draw_oam();
};

} // namespace gameboy

#endif  //GAMEBOY_PPU_DEBUGGER_H