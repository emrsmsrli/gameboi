#ifndef GAMEBOY_MMU_H
#define GAMEBOY_MMU_H

#include <cstdint>
#include <vector>
#include <memory>

#include <util/observer.h>
#include <memory/controller/mbc.h>
#include <memory/addressfwd.h>

namespace gameboy {

class bus;

class mmu {
public:
    explicit mmu(observer<bus> bus);

    void initialize();

    void write(const address16& address, uint8_t data);
    [[nodiscard]] uint8_t read(const address16& address) const;

private:
    observer<bus> bus_;

    uint8_t selected_work_ram_bank_ = 0;

    std::vector<uint8_t> work_ram_;
    std::vector<uint8_t> high_ram_;
};

}

#endif //GAMEBOY_MMU_H
