//
// Created by Emre Şimşirli on 25.08.2019.
//

#ifndef GAMEBOY_MBC1_H
#define GAMEBOY_MBC1_H

#include "MBC.h"

/**
 * 16Mbit ROM/8KByte RAM
 * 4bit ROM/32KByte RAM
 *
 * the MBC1 defaults to 16Mbit ROM/8KByte RAM mode on power up.
 *
 * writing a value (XXXXXXXS - X = Don't care, S = Memory model select)
 * into 6000-7FFF area will select the memory model to use. S = 0 selects 16/8 mode. S = 1 selects 4/32 mode.
 *
 * writing a value (XXXBBBBB - X = Don't cares, B = bank select bits) into 2000-3FFF area will select
 * an appropriate ROM bank at 4000-7FFF. Values of 0    and 1 do the same thing and point to ROM bank 1
 *
 * rom bank 0 is not accessible from 4000-7FFF and can    only be read from 0000-3FFF
 *
 * memory model is set to 4/32:
 *     Writing a value (XXXXXXBB - X = Don't care, B = bank select bits) into 4000-5FFF area will
 *     select an appropriate RAM bank at A000-C000. Before you can read or write to a RAM bank you have to enable
 *     it by writing a XXXX1010 into 0000-1FFF area*. To disable RAM bank operations write any value but XXXX1010 into
 *     0000-1FFF area. Disabling a RAM bank probably protects that bank from false writes
 *     during power down of the GameBoy. (NOTE: Nintendo suggests values $0A to enable and $00 to disable RAM bank!!)
 * memory model is set to 16/8 mode:
 *     Writing a value (XXXXXXBB - X = Don't care, B = bank select bits) into 4000-5FFF area will set the
 *     two most significant ROM address lines.    * NOTE: The Super Smart Card doesn't require this
 *     operation because it's RAM bank is ALWAYS enabled.    Include this operation anyway to allow your code
 *     to work with both
 */
namespace gameboy::memory::controller {
    class MBC1 : public MBC {

    };
}

#endif //GAMEBOY_MBC1_H
