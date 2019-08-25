//
// Created by Emre Şimşirli on 25.08.2019.
//

#ifndef GAMEBOY_MBC5_H
#define GAMEBOY_MBC5_H

#include "MBC.h"

/**
 * it is similar to the MBC3 (but no RTC) but can    access up to 64mbits of ROM and up to 1mbit of RAM.
 * The lower 8 bits of the 9-bit rom bank select is    written to the 2000-2FFF area while the upper bit
 * is written to the least significant bit of the    3000-3FFF area.
 * Writing a value (XXXXBBBB - X = Don't care, B =    bank select bits) into 4000-5FFF area will select
 * an appropriate RAM bank at A000-BFFF if the cart    contains RAM. Ram sizes are 64kbit,256kbit, &    1mbit.
 * Also, this is the first MBC that allows rom bank 0    to appear in the 4000-7FFF range by writing $000
 * to the rom bank select
 */
namespace gameboy::memory::controller {
    class MBC5 : public MBC {

    };
}

#endif //GAMEBOY_MBC5_H
