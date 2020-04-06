#ifndef GAMEBOY_WAVE_PATTERN_DUTY_H
#define GAMEBOY_WAVE_PATTERN_DUTY_H

#include "gameboy/cpu/register8.h"

namespace gameboy {

/**
 * Bit 7-6 - Wave Pattern Duty (Read/Write)
 * Bit 5-0 - Sound length data (Write Only) (t1: 0-63)
 *
 * Wave Duty:
 *
 *   00: 12.5% ( _-------_-------_------- )
 *   01: 25%   ( __------__------__------ )
 *   10: 50%   ( ____----____----____---- ) (normal)
 *   11: 75%   ( ______--______--______-- )
 *
 * Sound Length = (64-t1)*(1/256) seconds
 * The Length value is used only if Bit 6 in NR14 is set.
 */
struct wave_pattern_duty {
    register8 reg;
};

} // namespace gameboy

#endif //GAMEBOY_WAVE_PATTERN_DUTY_H
