#ifndef GAMEBOY_BUS_H
#define GAMEBOY_BUS_H

#include <util/observer.h>

namespace gameboy {

class gameboy;
class cartridge;
class mmu;
class cpu;
class ppu;
class apu;

class bus {
public:
    explicit bus(const observer<gameboy> gb) noexcept
        : gb_{gb} {}

    [[nodiscard]] observer<cartridge> get_cartridge() const noexcept;
    [[nodiscard]] observer<mmu> get_mmu() const noexcept;
    [[nodiscard]] observer<cpu> get_cpu() const noexcept;
    [[nodiscard]] observer<ppu> get_ppu() const noexcept;
    [[nodiscard]] observer<apu> get_apu() const noexcept;

private:
    observer<gameboy> gb_;
};

} // namespace gameboy

#endif //GAMEBOY_BUS_H
