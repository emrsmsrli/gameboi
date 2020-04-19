#include <array>
#include <algorithm>

#include <magic_enum.hpp>

#include "debugger/apu_debugger.h"
#include "debugger/debugger_util.h"
#include "gameboy/apu/apu.h"
#include "imgui.h"

namespace gameboy {

apu_debugger::apu_debugger(const observer<gameboy::apu> apu) noexcept
    : apu_{apu}
{
    memory_editor_.ReadOnly = true;
    memory_editor_.Cols = 8;
}

void apu_debugger::draw() noexcept
{
    if(!ImGui::Begin("APU")) {
        ImGui::End();
        return;
    }

    if(ImGui::BeginTabBar("apuchannels")) {
        if(ImGui::BeginTabItem("Mixer")) {
            ImGui::Columns(2);
            ImGui::Text("power        %d", apu_->power_on_);

            ImGui::Separator();
            ImGui::TextColored(ImVec4{.5f, .5f, .5f, 1.f}, "NR50: %02X", apu_->control_.nr_50.value());
            ImGui::Separator();
            ImGui::Text("vol left     %02X",
                apu_->control_.terminal_volume<uint8_t>(audio::control::terminal::left));
            ImGui::NextColumn();
            ImGui::Text("vol right    %02X",
                apu_->control_.terminal_volume<uint8_t>(audio::control::terminal::right));
            ImGui::NextColumn();
            ImGui::Separator();

            ImGui::TextColored(ImVec4{.5f, .5f, .5f, 1.f}, "NR51: %02X", apu_->control_.nr_51.value());
            ImGui::Separator();

            for(int i = 0; i < 4; ++i) {
                ImGui::Text("ch%d left     %d", i + 1,
                    apu_->control_.channel_enabled_on_terminal(i, audio::control::terminal::left));
            }
            ImGui::NextColumn();
            for(int i = 0; i < 4; ++i) {
                ImGui::Text("ch%d right    %d", i + 1,
                    apu_->control_.channel_enabled_on_terminal(i, audio::control::terminal::right));
            }

            ImGui::Columns(1);
            ImGui::Separator();

            std::array<float, apu::sample_size> samples_f;
            std::transform(begin(apu_->sound_buffer_), end(apu_->sound_buffer_), begin(samples_f),
                [](const int16_t sample) {
                    return static_cast<float>(sample) / static_cast<float>(std::numeric_limits<int16_t>::max());
                });

            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();

            const auto draw_debug_channel = [&](const char* name, const auto& buffer) {
                ImGui::BeginGroup();
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(name);
                ImGui::PlotLines("",
                    buffer.data(), buffer.size(), apu_->buffer_fill_amount_,
                    nullptr, -0.1f, 1.0f, ImVec2(95.f, 160.f));
                ImGui::EndGroup();
                ImGui::SameLine();
            };

            draw_debug_channel("Channel1", apu_->sound_buffer_1_);
            draw_debug_channel("Channel2", apu_->sound_buffer_2_);
            draw_debug_channel("Channel3", apu_->sound_buffer_3_);
            draw_debug_channel("Channel4", apu_->sound_buffer_4_);
            ImGui::NewLine();

            ImGui::TextUnformatted("Sound buffer visualizer");
            ImGui::PlotLines("",
                samples_f.data(), samples_f.size(), apu_->buffer_fill_amount_,
                nullptr, -0.1f, 1.0f, ImVec2(400.f,160.f));

            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("Channel1")) {
            draw_pulse_channel(apu_->channel_1_, false);
            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("Channel2")) {
            draw_pulse_channel(apu_->channel_2_, true);
            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("Channel3")) {
            ImGui::Columns(2);
            ImGui::Text("timer        %04X", apu_->channel_3_.timer);
            ImGui::Text("output       %02X", apu_->channel_3_.output);
            ImGui::NextColumn();
            ImGui::Text("length       %04X", apu_->channel_3_.length_counter);
            ImGui::Text("sample idx   %02X", apu_->channel_3_.sample_index);
            ImGui::NextColumn();
            ImGui::Separator();
            ImGui::Text("enable       %d", apu_->channel_3_.enabled);
            ImGui::NextColumn();
            ImGui::Text("dac enable   %d", apu_->channel_3_.dac_enabled);
            ImGui::Columns(1);

            ImGui::Separator();
            ImGui::TextColored(ImVec4{.5f, .5f, .5f, 1.f}, "SoundLength: %02X", apu_->channel_3_.sound_length.value());
            ImGui::Separator();

            ImGui::TextColored(ImVec4{.5f, .5f, .5f, 1.f}, "OutputLevel: %02X", apu_->channel_3_.output_level.value());
            ImGui::Separator();

            draw_freq_data(apu_->channel_3_.frequency);
            ImGui::Separator();

            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("Wave RAM");
            ImGui::Separator();
            memory_editor_.DrawContents(
                apu_->channel_3_.wave_pattern.data(),
                apu_->channel_3_.wave_pattern.size(),
                0xFF30u);
            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("Channel4")) {
            ImGui::Columns(2);
            ImGui::Text("timer        %08X", apu_->channel_4_.timer);
            ImGui::Text("output       %02X", apu_->channel_4_.output);
            ImGui::Text("lfsr         %04X", apu_->channel_4_.lfsr);
            ImGui::NextColumn();
            ImGui::Text("length       %02X", apu_->channel_4_.length_counter);
            ImGui::Text("volume       %02X", apu_->channel_4_.volume);
            ImGui::NextColumn();
            ImGui::Separator();
            ImGui::Text("enable       %d", apu_->channel_4_.enabled);
            ImGui::NextColumn();
            ImGui::Text("dac enable   %d", apu_->channel_4_.dac_enabled);
            ImGui::NextColumn();
            ImGui::Columns(1);

            ImGui::Separator();
            ImGui::TextColored(ImVec4{.5f, .5f, .5f, 1.f}, "SoundLength: %02X", apu_->channel_4_.sound_length);
            ImGui::Separator();

            draw_envelope(apu_->channel_4_.envelope);
            ImGui::Separator();

            ImGui::TextColored(ImVec4{.5f, .5f, .5f, 1.f}, "PolyCounter: %02X",
                apu_->channel_4_.polynomial_counter.reg.value());
            ImGui::Text("shift clock freq    %02X",
                apu_->channel_4_.polynomial_counter.shift_clock_frequency());
            ImGui::Text("counter width       %d",
                apu_->channel_4_.polynomial_counter.has_7_bit_counter_width() ? 7 : 15);
            ImGui::Text("dividing ratio      %02X",
                apu_->channel_4_.polynomial_counter.dividing_ratio());
            ImGui::Separator();

            draw_freq_control(apu_->channel_4_.control);

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void apu_debugger::draw_freq_data(const audio::frequency_data& freq_data) const noexcept
{
    ImGui::TextColored(ImVec4{.5f, .5f, .5f, 1.f}, "Freq: %04X", freq_data.value());
    ImGui::Separator();

    ImGui::Text("freq low     %02X", freq_data.low.value());
    draw_freq_control(freq_data.freq_control);
}

void apu_debugger::draw_freq_control(const audio::frequency_control& freq_ctrl) const noexcept
{
    ImGui::Text("freq ctrl    %02X", freq_ctrl.reg.value());
    ImGui::Text("trigger      %d", freq_ctrl.should_restart());
    ImGui::Text("use counter  %d", freq_ctrl.use_counter());
}

void apu_debugger::draw_envelope(const audio::envelope& env) const noexcept
{
    ImGui::TextColored(ImVec4{.5f, .5f, .5f, 1.f}, "Envelope: %02X", env.reg.value());
    ImGui::Separator();

    ImGui::Columns(2);
    ImGui::Text("timer        %08X", env.timer);
    ImGui::NextColumn();
    ImGui::Text("period       %02X", env.period());
    ImGui::TextUnformatted("mode         "); ImGui::SameLine(0, 0);
    show_string_view(magic_enum::enum_name(env.get_mode()));
    ImGui::Text("init vol     %02X", env.initial_volume());
    ImGui::Columns(1);
}

void apu_debugger::draw_pulse_channel(const pulse_channel& channel, const bool no_sweep) const noexcept
{
    ImGui::Columns(2);
    ImGui::Text("timer        %04X", channel.timer);
    ImGui::Text("volume       %02X", channel.volume);
    ImGui::Text("wave idx     %02X", channel.waveform_index);
    ImGui::NextColumn();
    ImGui::Text("length       %02X", channel.length_counter);
    ImGui::Text("output       %02X", channel.output);
    ImGui::NextColumn();
    ImGui::Separator();
    ImGui::Text("enable       %d", channel.enabled);
    ImGui::NextColumn();
    ImGui::Text("dac enable   %d", channel.dac_enabled);
    ImGui::Columns(1);
    ImGui::Separator();

    if(!no_sweep) {
        ImGui::TextColored(ImVec4{.5f, .5f, .5f, 1.f}, "Sweep: %02X", channel.sweep.reg.value());
        ImGui::Separator();

        ImGui::Columns(2);
        ImGui::Text("enabled      %d", channel.sweep.enabled);
        ImGui::Text("timer        %04X", channel.sweep.timer);
        ImGui::Text("shadow       %04X", channel.sweep.shadow);
        ImGui::NextColumn();
        ImGui::Text("period       %02X", channel.sweep.period());
        ImGui::TextUnformatted("mode         "); ImGui::SameLine(0, 0);
        show_string_view(magic_enum::enum_name(channel.sweep.get_mode()));
        ImGui::Text("shift count  %02X", channel.sweep.shift_count());
        ImGui::Columns(1);
        ImGui::Separator();
    }

    ImGui::TextColored(ImVec4{.5f, .5f, .5f, 1.f}, "WaveData: %02X", channel.wave_data.reg.value());
    ImGui::Separator();

    ImGui::Columns(2);
    ImGui::Text("duty         %02X", channel.wave_data.duty()); ImGui::SameLine();
    ImGui::NextColumn();
    ImGui::Text("length       %02X", channel.wave_data.sound_length());
    ImGui::Columns(1);
    ImGui::Separator();

    draw_envelope(channel.envelope);
    ImGui::Separator();

    draw_freq_data(channel.frequency_data);
}

} // namespace gameboy
