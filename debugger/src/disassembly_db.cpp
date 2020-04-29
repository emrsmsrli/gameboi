#include "debugger/disassembly_db.h"

#include <fmt/format.h>

#include "gameboy/bus.h"
#include "gameboy/cartridge.h"
#include "gameboy/memory/memory_constants.h"
#include "gameboy/memory/mmu.h"

namespace gameboy::instruction {

disassembly_db::disassembly_db(
    observer<bus> bus,
    std::string_view name,
    const std::vector<uint8_t>& data) noexcept
    : bus_{bus},
      data_{data},
      name_{name},
      bank_size_{
        [&]() -> size_t {
            if(name == name_rom) {
                return 16_kb;
            }

            if(name == name_wram) {
                return 4_kb;
            }

            return wram_range.size();
        }()
      }
{
    if(name == name_rom) {
        generate_disassembly(0u, 0x0104u);
        generate_disassembly(0x0150);
    } else {
        generate_disassembly(0u);
    }
}

void disassembly_db::on_write(const address16& addr, [[maybe_unused]] uint8_t data) noexcept
{
    if(!earliest_dirty_addr_ || addr.value() < earliest_dirty_addr_.value().value()) {
        earliest_dirty_addr_ = addr;

        if(name_ == name_rom) {
            earliest_dirty_bank_ = bus_->get_cartridge()->rom_bank(addr);
        }

        if(name_ == name_wram) {
            constexpr address_range first_wram{0xC000u, 0xCFFFu};
            const auto address = echo_range.has(addr) ? addr - (*begin(echo_range) - *begin(wram_range)) : addr;
            earliest_dirty_bank_ = first_wram.has(address) ? 0u : bus_->get_mmu()->wram_bank_;
        }

        earliest_dirty_bank_ = 0u;
    }

    ++dirty_address_count_;
}

const std::vector<disassembly>& disassembly_db::get() noexcept
{
    if(earliest_dirty_addr_) {
        const auto find_disassembly = [&](const address16& addr, const uint32_t bank) {
          return std::find_if(begin(disassemblies_), end(disassemblies_),
              [&](const instruction::disassembly& disassembly) {
                  return disassembly.address == addr && disassembly.bank == bank;
              });
        };

        for(auto tries = 0; tries < 3; ++tries) {
            auto bank = earliest_dirty_bank_;
            auto addr = earliest_dirty_addr_.value() - tries;

            if(name_ == name_wram) {
                if(addr.value() < *begin(wram_range)) {
                    generate_disassembly(0u);
                    return disassemblies_;
                }

                constexpr address_range first_bank{0xC000, 0xCFFF};
                if(first_bank.has(addr) && !first_bank.has(earliest_dirty_addr_.value())) {
                    --bank;
                }
            } else if(name_ == name_hram && addr.value() < *begin(hram_range)) {
                generate_disassembly(0u);
                return disassemblies_;
            }

            if(auto it = find_disassembly(addr, bank); it != end(disassemblies_)) {
                auto& existing_disassembly = *it;

                const auto physical_addr = ((addr.value() - base_address()) % bank_size_) + bank * bank_size_;
                const auto [new_phys_addr, new_disassembly] = disassemble(physical_addr);

                if(dirty_address_count_ == 1 && existing_disassembly.info.length == new_disassembly.info.length) {
                    existing_disassembly = new_disassembly;
                } else {
                    disassemblies_.erase(it, end(disassemblies_));
                    disassemblies_.push_back(new_disassembly);

                    generate_disassembly(new_phys_addr);
                }

                dirty_address_count_ = 0;
                earliest_dirty_addr_ = std::nullopt;
                break;
            }
        }
    }

    return disassemblies_;
}

std::pair<size_t, disassembly> disassembly_db::disassemble(size_t physical_addr) noexcept
{
    const uint32_t bank = physical_addr / bank_size_;
    const auto virtual_address = [&]() -> uint16_t {
        return (bank == 0 ? physical_addr : (physical_addr % bank_size_) + bank_size_) + base_address();
    }();

    auto [instruction_info, is_cgb] = data_[physical_addr] == 0xCBu
        ? std::make_pair(instruction::extended_instruction_set[data_[physical_addr + 1]], true)
        : std::make_pair(instruction::standard_instruction_set[data_[physical_addr]], false);

    if(instruction_info.length == 0) {
        instruction_info.length = 1;
    }

    std::string representation;
    if(instruction_info.length == 1) {
        representation = fmt::format("{}{}:{:04X} | {}",
                name_, bank, virtual_address, instruction_info.mnemonic);
    } else {
        uint16_t data = 0;
        for(auto d_i = 0; d_i < instruction_info.length - 1; ++d_i) {
            data |= data_[physical_addr + d_i + 1] << (d_i * 8u);
        }

        representation = fmt::format("{}{}:{:04X} | {}", name_, bank, virtual_address,
          fmt::format(instruction_info.mnemonic.data(), data));
    }

    physical_addr += instruction_info.length;
    if(is_cgb) {
        physical_addr += 1;
    }

    return std::make_pair(physical_addr, instruction::disassembly{
        bank,
        make_address(virtual_address),
        instruction_info,
        std::move(representation)
    });
}

void disassembly_db::generate_disassembly(const size_t start, const size_t end) noexcept
{
    for(size_t physical_addr = start; physical_addr < end;) {
        const auto [new_physical_addr, disassembly] = disassemble(physical_addr);
        physical_addr = new_physical_addr;
        disassemblies_.push_back(disassembly);
    }
}

} // namespace gameboy::instruction
