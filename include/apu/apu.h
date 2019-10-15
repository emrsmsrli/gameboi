#ifndef GAMEBOY_APU_H
#define GAMEBOY_APU_H

#include <util/observer.h>

namespace gameboy {

struct bus;

class apu {
public:
    explicit apu(observer<bus> bus)
        : bus_(bus) {}

private:
    observer<bus> bus_;
};

}

#endif //GAMEBOY_APU_H
