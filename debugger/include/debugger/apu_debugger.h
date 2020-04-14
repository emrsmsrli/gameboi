#ifndef GAMEBOY_APU_DEBUGGER_H
#define GAMEBOY_APU_DEBUGGER_H

#include "gameboy/util/observer.h"
#include "imgui_memory_editor/imgui_memory_editor.h"

namespace gameboy {

class apu;
struct pulse_channel;
struct envelope;
struct frequency_data;
struct frequency_control;

class apu_debugger {
public:
    explicit apu_debugger(observer<apu> apu) noexcept;
    void draw() noexcept;

private:
    observer<apu> apu_;
    MemoryEditor memory_editor_;

    void draw_freq_data(const frequency_data& freq_data) const noexcept;
    void draw_freq_control(const frequency_control& freq_ctrl) const noexcept;
    void draw_envelope(const envelope& env) const noexcept;
    void draw_pulse_channel(const pulse_channel& channel, bool no_sweep) const noexcept;
};

} // namespace gameboy

#endif //GAMEBOY_APU_DEBUGGER_H
