#include <sstream>

#include <magic_enum.hpp>
#include <fmt/format.h>

#include "debugger/cpu_debugger.h"
#include "debugger/cartridge_debugger.h"
#include "gameboy/cpu/cpu.h"
#include "gameboy/memory/memory_constants.h"
#include "gameboy/bus.h"
#include "gameboy/ppu/ppu.h"
#include "gameboy/memory/mmu.h"
#include "gameboy/cartridge.h"
#include "imgui.h"

using namespace magic_enum::bitwise_operators;

gameboy::cpu_debugger::cpu_debugger(const observer<cpu> cpu) noexcept
    : cpu_{cpu} {}

void gameboy::cpu_debugger::draw() noexcept
{
    if(!ImGui::Begin("CPU")) {
        ImGui::End();
        return;
    }

    if(ImGui::BeginTabBar("cputabs")) {
        if(ImGui::BeginTabItem("Info")) {
            ImGui::Text("Total cycles: %lld", cpu_->total_cycles_);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            draw_registers();
            ImGui::Separator();

            ImGui::Spacing();
            ImGui::Spacing();

            draw_interrupts();

            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("Breakpoints")) {
            ImGui::BeginChild("breakpointchildhack");

            static constexpr std::array breakpoint_types{"Execution breakpoint", "Access breakpoint"};
            static int breakpoint_type = 0;
            ImGui::Combo("", &breakpoint_type, breakpoint_types.data(), breakpoint_types.size());

            ImGui::Spacing();
            ImGui::Spacing();

            if(breakpoint_type == 0) {
                draw_execution_breakpoints();
            } else if(breakpoint_type == 1) {
                draw_access_breakpoints();
            }

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("Last 100 Instrs")) {
            draw_last_100_instructions();
            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("Call Stack")) {
            draw_call_stack();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    
    ImGui::End();
    ImGui::ShowDemoWindow();
}

void gameboy::cpu_debugger::draw_execution_breakpoints() noexcept
{
    static std::array<char, 5> address_buf{};
    static std::array<char, 4> bank_buf{};

    ImGui::PushItemWidth(120);
    ImGui::InputText("addr", address_buf.data(), address_buf.size(),
        ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
    ImGui::SameLine();
    ImGui::InputText("bank", bank_buf.data(), bank_buf.size());
    ImGui::PopItemWidth();

    if(ImGui::Button("OK")) {
        if(std::strlen(address_buf.data()) != 0) {
            execution_breakpoint breakpoint;
            breakpoint.address = std::strtoul(address_buf.data(), nullptr, 16);

            const auto add_if_not_exists = [&](const execution_breakpoint& b) {
                auto disabled_breakpoint = b;
                disabled_breakpoint.enabled = false;

                if(has_execution_breakpoint(b) || has_execution_breakpoint(disabled_breakpoint)) {
                    return;
                }

                execution_breakpoints_.push_back(b);
            };

            if(std::strlen(bank_buf.data()) != 0) {
                breakpoint.bank = std::atoi(bank_buf.data());

                if(breakpoint_bank_valid(breakpoint.address, breakpoint.bank)) {
                    add_if_not_exists(breakpoint);
                }
            } else {
                add_if_not_exists(breakpoint);
            }
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    if(!execution_breakpoints_.empty()) {
        ImGui::BeginChild("exebreak");

        auto to_delete = -1;
        ImGuiListClipper clipper(execution_breakpoints_.size());
        while(clipper.Step()) {
            for(auto i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                auto& [address, bank, enabled] = execution_breakpoints_[i];

                // fixme does not work if displaying more than one item and clicked the last one
                if(ImGui::Button("X")) {
                    to_delete = i;
                }

                ImGui::SameLine(0, 5);
                ImGui::Checkbox("", &enabled);

                ImGui::SameLine(0, 10);
                if(bank == execution_breakpoint::any_bank) {
                    ImGui::Text("any:%04X", address.value());
                } else {
                    ImGui::Text("%3d:%04X", bank, address.value());
                }
            }
        }

        if(to_delete != -1) {
            execution_breakpoints_.erase(begin(execution_breakpoints_) + to_delete);
        }

        ImGui::EndChild();
    } else {
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::TextUnformatted("No breakpoints");
    }
}

void gameboy::cpu_debugger::draw_access_breakpoints() noexcept
{
    static constexpr std::array access_types{"Read", "Write", "Read&Write"};
    static int access_type = 0;
    static std::array<char, 5> address_lo_buf{};
    static std::array<char, 5> address_hi_buf{};
    static std::array<char, 3> data_buf{};

    ImGui::Combo("Access Type", &access_type, access_types.data(), access_types.size());

    ImGui::PushItemWidth(120);
    ImGui::InputText("addr low", address_lo_buf.data(), address_lo_buf.size(),
        ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);

    ImGui::SameLine();
    ImGui::InputText("addr hi", address_hi_buf.data(), address_hi_buf.size(),
        ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);

    if(access_type == 1 || access_type == 2) {
        ImGui::InputText("data", data_buf.data(), data_buf.size(),
            ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
    }

    ImGui::PopItemWidth();

    if(ImGui::Button("OK")) {
        if(std::strlen(address_lo_buf.data()) != 0) {
            access_breakpoint breakpoint;
            const auto range_lo = std::strtoul(address_lo_buf.data(), nullptr, 16);
            auto range_hi = range_lo;

            if(std::strlen(address_hi_buf.data()) != 0) {
                range_hi = std::strtoul(address_hi_buf.data(), nullptr, 16);
            }

            breakpoint.range = address_range(range_lo, range_hi);
            breakpoint.access_type = static_cast<access_breakpoint::type>(access_type);
            if((access_type == 1 || access_type == 2) && std::strlen(data_buf.data()) != 0) {
                breakpoint.data = std::strtoul(data_buf.data(), nullptr, 16);
            }

            auto disabled_breakpoint = breakpoint;
            disabled_breakpoint.enabled = false;

            if(has_access_breakpoint(breakpoint) || has_access_breakpoint(disabled_breakpoint)) {
                return;
            }

            access_breakpoints_.push_back(breakpoint);
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    if(!access_breakpoints_.empty()) {
        ImGui::BeginChild("accessbreak");

        auto to_delete = -1;
        ImGuiListClipper clipper(access_breakpoints_.size());
        while(clipper.Step()) {
            for(auto i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                auto& [range, access_type, data, enabled] = access_breakpoints_[i];

                // fixme does not work if displaying more than one item and clicked the last one
                if(ImGui::Button("X")) {
                    to_delete = i;
                }

                ImGui::SameLine(0, 5);
                ImGui::Checkbox("", &enabled);

                std::ostringstream fmtstr;
                if(range.size() == 1) {
                    fmtstr << fmt::format("[   {:04X}   ]:", *begin(range));
                } else {
                    fmtstr << fmt::format("[{:04X}][{:04X}]:", *begin(range), *end(range) - 1);
                }

                fmtstr << [](access_breakpoint::type type) {
                    switch(type) {
                        case access_breakpoint::type::read:         return "r ";
                        case access_breakpoint::type::write:        return "w ";
                        case access_breakpoint::type::read_write:   return "rw";
                    }
                }(access_type) << ':';

                if(data) {
                    fmtstr << fmt::format("{:02X}", *data);
                } else {
                    fmtstr << "NA";
                }

                ImGui::SameLine(0, 10);
                ImGui::TextUnformatted(fmtstr.str().c_str());
            }
        }

        if(to_delete != -1) {
            access_breakpoints_.erase(begin(access_breakpoints_) + to_delete);
        }

        ImGui::EndChild();
    } else {
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::TextUnformatted("No breakpoints");
    }
}

void gameboy::cpu_debugger::draw_registers() const noexcept
{
    ImGui::Columns(3, "registers and flags", true);
    
    ImGui::Text("Registers"); ImGui::NextColumn();
    ImGui::Text("SP and PC"); ImGui::NextColumn();
    ImGui::Text("Flags"); ImGui::NextColumn();
    ImGui::Separator();

    ImGui::Text("%s: %04X", "AF", cpu_->a_f_.value());
    ImGui::Text("%s: %04X", "BC", cpu_->b_c_.value());
    ImGui::Text("%s: %04X", "DE", cpu_->d_e_.value());
    ImGui::Text("%s: %04X", "HL", cpu_->h_l_.value());

    ImGui::NextColumn();

    ImGui::Text("SP: %04X", cpu_->stack_pointer_.value());
    ImGui::Text("PC: %04X", cpu_->program_counter_.value());
    
    ImGui::NextColumn();

    ImGui::Text("Z: %d", cpu_->test_flag(cpu::flag::zero));
    ImGui::Text("N: %d", cpu_->test_flag(cpu::flag::negative));
    ImGui::Text("H: %d", cpu_->test_flag(cpu::flag::half_carry));
    ImGui::Text("C: %d", cpu_->test_flag(cpu::flag::carry));

    ImGui::Columns(1);
}

void gameboy::cpu_debugger::draw_interrupts() const noexcept
{
    ImGui::Text("speed     %d, preparing: %d", bit::extract(cpu_->key_1_, 7u), bit::extract(cpu_->key_1_, 0u));
    ImGui::Text("stop flag %d", cpu_->is_stopped_);
    ImGui::Text("halt flag %d", cpu_->is_halted_);
    ImGui::Text("ime: %s", 
        cpu_->interrupts_enabled() 
            ? "enabled" 
            : "disabled");
    ImGui::Separator();
    
    ImGui::Columns(2, "interrupts", true);
    
    ImGui::Text("interrupt enable %02X", static_cast<uint8_t>(cpu_->interrupt_enable_)); ImGui::NextColumn();
    ImGui::Text("interrupt flags %02X", static_cast<uint8_t>(cpu_->interrupt_flags_));  ImGui::NextColumn();
    ImGui::Separator();

    const auto interrupt_enable = [&](auto interrupt) { return (cpu_->interrupt_enable_ & interrupt) == interrupt; };
    ImGui::Text("lcd_vblank: %d", interrupt_enable(interrupt::lcd_vblank));
    ImGui::Text("lcd_stat:   %d", interrupt_enable(interrupt::lcd_stat));
    ImGui::Text("timer:      %d", interrupt_enable(interrupt::timer));
    ImGui::Text("serial:     %d", interrupt_enable(interrupt::serial));
    ImGui::Text("joypad:     %d", interrupt_enable(interrupt::joypad));

    ImGui::NextColumn();
    
    const auto interrupt_flag = [&](auto interrupt) { return (cpu_->interrupt_flags_ & interrupt) == interrupt; };
    ImGui::Text("lcd_vblank: %d", interrupt_flag(interrupt::lcd_vblank));
    ImGui::Text("lcd_stat:   %d", interrupt_flag(interrupt::lcd_stat));
    ImGui::Text("timer:      %d", interrupt_flag(interrupt::timer));
    ImGui::Text("serial:     %d", interrupt_flag(interrupt::serial));
    ImGui::Text("joypad:     %d", interrupt_flag(interrupt::joypad));

    ImGui::Columns(1);
}

void gameboy::cpu_debugger::draw_last_100_instructions() const noexcept
{
    if(ImGui::BeginChild("cpulast100")) {
        ImGuiListClipper clipper(last_executed_instructions_.size());
        while(clipper.Step()) {
            for(auto i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                ImGui::TextUnformatted(last_executed_instructions_[i].c_str());
            }
        }
        ImGui::EndChild();
    }
}

void gameboy::cpu_debugger::draw_call_stack() const noexcept
{
    ImGuiListClipper clipper(call_stack_.size());
    while(clipper.Step()) {
        for(auto i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
            ImGui::Text("%04X", call_stack_[i].value());
        }
    }
}

void gameboy::cpu_debugger::on_instruction(
    const address16& addr,
    const instruction::info& info,
    const uint16_t data) noexcept
{
    const auto starts_with = [](const std::string_view str, const std::string_view what) {
        return str.compare(0, what.size(), what) == 0;
    };

    if(starts_with(info.mnemonic, "CALL")) {
        call_stack_.push_back(addr);
    } else if(starts_with(info.mnemonic, "RET")) {
        if(!call_stack_.empty()) {
            call_stack_.pop_back();
        }
    }

    if(last_executed_instructions_.size() == 100) {
        last_executed_instructions_.erase(begin(last_executed_instructions_));
    }

    last_executed_instructions_.push_back(fmt::format("{:04X}: {}", addr.value(), fmt::format(info.mnemonic.data(), data)));
}

gameboy::register16 gameboy::cpu_debugger::get_pc() const noexcept
{
    return cpu_->program_counter_;
}

bool gameboy::cpu_debugger::has_execution_breakpoint() const
{
    const auto pc_addr = make_address(get_pc());
    const auto contains = [&](int bank) {
      return has_execution_breakpoint(execution_breakpoint{pc_addr, bank});
    };

    if(rom_range.has(pc_addr)) {
        return contains(cpu_->bus_->get_cartridge()->rom_bank(pc_addr))
            || contains(execution_breakpoint::any_bank);
    }

    if(xram_range.has(pc_addr)) {
        return contains(cpu_->bus_->get_cartridge()->ram_bank()) || contains(execution_breakpoint::any_bank);
    }

    static constexpr address_range first_wram{0xC000u, 0xCFFFu};
    if(wram_range.has(pc_addr)) {
        return contains(first_wram.has(pc_addr) ? 0 : cpu_->bus_->get_mmu()->wram_bank_)
            || contains(execution_breakpoint::any_bank);
    }

    if(echo_range.has(pc_addr)) {
        const auto addr = pc_addr - (*begin(echo_range) - *begin(wram_range));
        static constexpr address_range first_wram{0xC000u, 0xCFFFu};
        return contains(first_wram.has(addr) ? 0 : cpu_->bus_->get_mmu()->wram_bank_)
            || contains(execution_breakpoint::any_bank);
    }

    if(hram_range.has(pc_addr)) {
        return contains(0) || contains(execution_breakpoint::any_bank);
    }

    return false;
}

bool gameboy::cpu_debugger::has_execution_breakpoint(const execution_breakpoint& breakpoint) const noexcept
{
    return std::find(
        cbegin(execution_breakpoints_),
        cend(execution_breakpoints_),
        breakpoint)
    != cend(execution_breakpoints_);
}

bool gameboy::cpu_debugger::has_access_breakpoint(
    const address16& address,
    const access_breakpoint::type access_type,
    const std::optional<uint8_t> data,
    const bool enabled) const noexcept
{
    return std::find_if(
        cbegin(access_breakpoints_),
        cend(access_breakpoints_),
        [&](const access_breakpoint& breakpoint) {
            return breakpoint.range.has(address)
                && breakpoint.access_type == access_type
                && breakpoint.data == data
                && breakpoint.enabled == enabled;
        })
    != cend(access_breakpoints_);
}

bool gameboy::cpu_debugger::has_access_breakpoint(const access_breakpoint& breakpoint) const noexcept
{
    return std::find(
        cbegin(access_breakpoints_),
        cend(access_breakpoints_),
        breakpoint)
    != cend(access_breakpoints_);
}

bool gameboy::cpu_debugger::breakpoint_bank_valid(const address16& addr, int bank) const noexcept
{
    if(rom_range.has(addr)) {
        return cpu_->bus_->get_cartridge()->rom_bank_count() > bank;
    }

    if(xram_range.has(addr)) {
        return cpu_->bus_->get_cartridge()->ram_bank_count() > bank;
    }

    static constexpr address_range first_wram{0xC000u, 0xCFFFu};
    if(wram_range.has(addr)) {
        const auto max_banks = cpu_->bus_->get_cartridge()->cgb_enabled() ? 8u : 2u;
        return first_wram.has(addr)
            ? bank == 0
            : bank != 0 && max_banks > bank;
    }

    if(echo_range.has(addr)) {
        const auto wram_addr = addr - (*begin(echo_range) - *begin(wram_range));
        const auto max_banks = cpu_->bus_->get_cartridge()->cgb_enabled() ? 8u : 2u;
        return first_wram.has(wram_addr)
               ? bank == 0
               : bank != 0 && max_banks > bank;
    }

    if(hram_range.has(addr)) {
        return bank == 0;
    }

    return false;
}
