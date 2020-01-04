#ifndef GAMEBOY_PPU_DEBUGGER_H
#define GAMEBOY_PPU_DEBUGGER_H

#include "gameboy/util/observer.h"

namespace gameboy {

class ppu;

class ppu_debugger {
public:
    explicit ppu_debugger(observer<ppu> ppu) noexcept;
   
    void draw() const noexcept;

private:
    observer<ppu> ppu_;

    void draw_registers() const noexcept;
    void draw_lcdc_n_stat() const;
    void draw_palettes() const;
    void draw_dma() const noexcept;
};

} // namespace gameboy

#endif  //GAMEBOY_PPU_DEBUGGER_H