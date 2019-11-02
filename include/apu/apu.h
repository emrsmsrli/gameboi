#ifndef GAMEBOY_APU_H
#define GAMEBOY_APU_H

#include <util/observer.h>

namespace gameboy {

class bus;

class apu {
public:
    explicit apu(observer<bus> bus)
        : bus_{bus} {}

private:
    observer<bus> bus_;
};

} // namespace gameboy

#endif //GAMEBOY_APU_H
