#ifndef GAMEBOY_MBC3_H
#define GAMEBOY_MBC3_H

#include <memory/controller/MBC.h>

/**
 * this controller is similar to MBC1 except it    accesses all 16mbits of ROM without requiring any
 * writes to the 4000-5FFF area.    Writing a value (XBBBBBBB - X = Don't care, B =    bank select bits)
 * into 2000-3FFF area will select    an appropriate ROM bank at 4000-7FFF.
 * Also, this MBC has a built-in battery-backed Real    Time Clock (RTC) not found in any other MBC. Some
 * MBC3 carts do not support it (WarioLand II non    color version) but some do (Harvest Moon/Japanese    version.)
 */
namespace gameboy::memory::controller {
    class MBC3 : public MBC {

    };
}

#endif //GAMEBOY_MBC3_H
