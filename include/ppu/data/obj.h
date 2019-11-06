#ifndef GAMEBOY_OBJ_H
#define GAMEBOY_OBJ_H

#include <cstdint>

#include <util/mathutil.h>

namespace gameboy {

/*

Byte2 - Tile/Pattern Number
Specifies the sprites Tile Number (00-FF). This (unsigned) value selects a tile from memory at 8000h-8FFFh.
 In CGB Mode this could be either in VRAM Bank 0 or 1, depending on Bit 3 of the following byte.
In 8x16 mode, the lower bit of the tile number is ignored.
 Ie. the upper 8x8 tile is "NN AND FEh", and the lower 8x8 tile is "NN OR 01h".

Sprite Priorities and Conflicts
When sprites with different x coordinate values overlap, the one with the smaller x coordinate (closer to the left)
 will have priority and appear above any others. This applies in Non CGB Mode only.
When sprites with the same x coordinate values overlap, they have priority according to table ordering. (i.e. $FE00
 - highest, $FE04 - next highest, etc.) In CGB Mode priorities are always assigned like this.

Only 10 sprites can be displayed on any one line. When this limit is exceeded, the lower priority sprites (priorities
 listed above) won't be displayed. To keep unused sprites from affecting onscreen sprites set their Y coordinate to
 Y=0 or Y=>144+16. Just setting the X coordinate to X=0 or X=>160+8 on a sprite will hide it but it will still affect
 other sprites sharing the same lines.
*/
struct obj {
    uint8_t y;
    uint8_t x;
    uint8_t tile_number;
    uint8_t attributes;

    /**
     * OBJ-to-BG Priority (0=OBJ Above BG, 1=OBJ Behind BG color 1-3)
     * (Used for both BG and Window. BG color 0 is always behind OBJ)
     */
    [[nodiscard]] bool prioritized() const noexcept { return !bit_test(attributes, 7u); }
    [[nodiscard]] bool v_flipped() const noexcept { return bit_test(attributes, 6u); }
    [[nodiscard]] bool h_flipped() const noexcept { return bit_test(attributes, 5u); }
    [[nodiscard]] uint8_t gb_palette_index() const noexcept { return bit_test(attributes, 4u) ? 0x1u : 0x0u; }
    [[nodiscard]] uint8_t vram_bank() const noexcept { return bit_test(attributes, 3u) ? 0x1u : 0x0u; } // todo cgb only
    [[nodiscard]] uint8_t cgb_palette_index() const noexcept { return mask(attributes, 0x7u); }
};

} // namespace gameboy

#endif //GAMEBOY_OBJ_H
