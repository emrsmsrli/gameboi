#include <iostream>
#include <gameboy.h>

int main(int /*argc*/, char** /*argv*/)
{
    gameboy::gameboy gb("rom/path.rom");
    gb.start();

    return 0;
}