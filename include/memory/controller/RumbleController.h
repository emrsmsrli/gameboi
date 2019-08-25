//
// Created by Emre Şimşirli on 25.08.2019.
//

#ifndef GAMEBOY_RUMBLE_H
#define GAMEBOY_RUMBLE_H

/**
 * Rumble carts use an MBC5 memory bank controller.
 * Rumble carts can only have up to 256kbits of RAM.
 * The highest RAM address line that allows 1mbit of
 * RAM on MBC5 non-rumble carts is used as the motor
 * on/off for the rumble cart.
 *
 * Writing a value (XXXXMBBB - X = Don't care, M = motor, B = bank select bits) into 4000-5FFF area
 * will select an appropriate RAM bank at A000-BFFF
 * if the cart contains RAM. RAM sizes are 64kbit or
 * 256kbits. To turn the rumble motor on set M = 1, M = 0 turns it off
 */
namespace gameboy::memory::controller {
    class Rumble : public MBC {

    };
}

#endif //GAMEBOY_RUMBLE_H
