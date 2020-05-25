#ifndef GAMEBOY_DEBUGGER_H
#define GAMEBOY_DEBUGGER_H

#include <memory>
#include <array>

#include <SFML/Graphics.hpp>
#include <spdlog/logger.h>

#include "debugger/apu_debugger.h"
#include "debugger/cartridge_debugger.h"
#include "debugger/cpu_debugger.h"
#include "debugger/disassembly_db.h"
#include "debugger/disassembly_view.h"
#include "debugger/joypad_debugger.h"
#include "debugger/memory_bank_debugger.h"
#include "debugger/ppu_debugger.h"
#include "debugger/timer_debugger.h"
#include "gameboy/util/observer.h"

namespace gameboy {

class bus;
class gameboy;

class debugger {
public:
    explicit debugger(observer<gameboy> gb);
    ~debugger();
    debugger(const debugger&) = delete;
    debugger(debugger&&) = delete;

    debugger& operator=(const debugger&) = delete;
    debugger& operator=(debugger&&) = delete;

    void tick();
    void on_instruction(const address16& addr, const instruction::info& info, uint16_t data) noexcept;
    void on_write_access(const address16& addr, uint8_t data) noexcept;
    void on_read_access(const address16& addr) noexcept;

    [[nodiscard]] bool has_focus() const noexcept { return window_.hasFocus(); }

private:
    observer<gameboy> gb_;
    observer<bus> bus_;
    apu_debugger apu_debugger_;
    cpu_debugger cpu_debugger_;
    cartridge_debugger cartridge_debugger_;
    ppu_debugger ppu_debugger_;
    timer_debugger timer_debugger_;
    joypad_debugger joypad_debugger_;
    disassembly_view disassembly_view_;
    memory_bank_debugger memory_bank_debugger_;

    size_t last_frame_time_idx_ = 0u;
    std::array<float, 100> last_frame_times_{};

    std::shared_ptr<spdlog::logger> logger_;

    sf::Clock delta_clock_;
    sf::RenderWindow window_;

    [[nodiscard]] bool has_execution_breakpoint()
    {
        const auto has_bp = cpu_debugger_.has_execution_breakpoint();
        if(has_bp) {
            logger_->info("breakpoint hit: program counter at {:04X}", cpu_debugger_.get_pc().value());
        }
        return has_bp;
    }
    [[nodiscard]] bool has_read_access_breakpoint(const address16& address)
    {
        const auto has_bp =
          cpu_debugger_.has_access_breakpoint(address, cpu_debugger::access_breakpoint::type::read) ||
            cpu_debugger_.has_access_breakpoint(address, cpu_debugger::access_breakpoint::type::read_write);
        if(has_bp) {
            logger_->info("breakpoint hit: read access at {:04X}", address.value());
        }
        return has_bp;
    }

    [[nodiscard]] bool has_write_access_breakpoint(const address16& address, const uint8_t data)
    {
        const auto has_bp =
          cpu_debugger_.has_access_breakpoint(address, cpu_debugger::access_breakpoint::type::write, data) ||
            cpu_debugger_.has_access_breakpoint(address, cpu_debugger::access_breakpoint::type::read_write, data) ||
            cpu_debugger_.has_access_breakpoint(address, cpu_debugger::access_breakpoint::type::write) ||
            cpu_debugger_.has_access_breakpoint(address, cpu_debugger::access_breakpoint::type::read_write);
        if(has_bp) {
            logger_->info("breakpoint hit: write access at {:04X} with {:02X}", address.value(), data);
        }
        return has_bp;
    }
};

} // namespace gameboy

#endif //GAMEBOY_DEBUGGER_H
