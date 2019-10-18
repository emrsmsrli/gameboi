#ifndef GAMEBOY_BUS_H
#define GAMEBOY_BUS_H

#include <util/observer.h>

namespace gameboy {

class cartridge;
class mmu;
class cpu;
class ppu;
class apu;

class bus {
    friend class gameboy;

public:
    explicit bus(observer<cartridge> cartridge) noexcept
        : cartridge_(cartridge) {}

    [[nodiscard]] const observer<cartridge>& get_cartridge() const noexcept { return cartridge_; }
    [[nodiscard]] const observer<mmu>& get_mmu() const noexcept { return mmu_; }
    [[nodiscard]] const observer<cpu>& get_cpu() const noexcept { return cpu_; }
    [[nodiscard]] const observer<ppu>& get_ppu() const noexcept { return ppu_; }
    [[nodiscard]] const observer<apu>& get_apu() const noexcept { return apu_; }

    [[nodiscard]] observer<cartridge>& get_cartridge() noexcept { return cartridge_; }
    [[nodiscard]] observer<mmu>& get_mmu() noexcept { return mmu_; }
    [[nodiscard]] observer<cpu>& get_cpu() noexcept { return cpu_; }
    [[nodiscard]] observer<ppu>& get_ppu() noexcept { return ppu_; }
    [[nodiscard]] observer<apu>& get_apu() noexcept { return apu_; }

private:
    observer<cartridge> cartridge_;
    observer<mmu> mmu_;
    observer<cpu> cpu_;
    observer<ppu> ppu_;
    observer<apu> apu_;
};

}

#endif //GAMEBOY_BUS_H
