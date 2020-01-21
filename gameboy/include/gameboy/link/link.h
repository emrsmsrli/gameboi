#ifndef GAMEBOY_LINK_H
#define GAMEBOY_LINK_H

#include <cstdint>

#include "gameboy/util/observer.h"
#include "gameboy/cpu/register8.h"

namespace gameboy {

class bus;

class link {
public:
    explicit link(observer<bus> bus) noexcept;

    void tick(uint8_t cycles) noexcept;

private:
    observer<bus> bus_;

    register8 sb_;
    register8 sc_;
    
    void on_sb_write(const address16&, uint8_t data) noexcept;
    [[nodiscard]] uint8_t on_sb_read(const address16&) const noexcept;

    void on_sc_write(const address16&, uint8_t data) noexcept;
    [[nodiscard]] uint8_t on_sc_read(const address16&) const noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_LINK_H