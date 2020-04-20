#ifndef GAMEBOY_LINK_H
#define GAMEBOY_LINK_H

#include <cstdint>

#include "gameboy/cpu/register8.h"
#include "gameboy/util/delegate.h"
#include "gameboy/util/observer.h"

namespace gameboy {

class bus;

class link {
public:
    enum class mode {
        external,
        internal
    };

    using transfer_func = delegate<uint8_t(uint8_t)>;

    explicit link(observer<bus> bus) noexcept;

    void tick(uint8_t cycles) noexcept;

    void on_transfer_master(const transfer_func on_transfer) { on_transfer_ = on_transfer; }
    uint8_t on_transfer_slave(uint8_t data) noexcept;

private:
    observer<bus> bus_;

    register8 sb_;
    register8 sc_;

    uint16_t shift_clock_;
    uint8_t shift_counter_;
    
    transfer_func on_transfer_;
    
    void on_sb_write(const address16&, uint8_t data) noexcept;
    [[nodiscard]] uint8_t on_sb_read(const address16&) const noexcept;

    void on_sc_write(const address16&, uint8_t data) noexcept;
    [[nodiscard]] uint8_t on_sc_read(const address16&) const noexcept;

    [[nodiscard]] bool is_transferring() const noexcept;
    [[nodiscard]] uint8_t clock_rate() const noexcept;
    [[nodiscard]] mode clock_mode() const noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_LINK_H