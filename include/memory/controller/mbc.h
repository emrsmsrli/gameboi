#ifndef GAMEBOY_MBC_H
#define GAMEBOY_MBC_H

#include <vector>
#include <cstdint>

#include <memory/address.h>

namespace gameboy {

struct cartridge;

class mbc {
public:
    explicit mbc(const std::vector<uint8_t>& rom, const cartridge& rom_header);
    virtual ~mbc() = default;

    void initialize();

    [[nodiscard]] virtual uint8_t read(const address16& virtual_address) const;
    virtual void write(const address16& virtual_address, uint8_t data);

protected:
    std::vector<uint8_t> memory_;

    uint32_t n_rom_banks_ = 0u;
    uint32_t n_video_ram_banks_ = 0u;
    uint32_t n_external_ram_banks_ = 0u;
    uint32_t n_working_ram_banks_ = 0u;

    uint32_t rom_bank_ = 0u;
    uint32_t ram_bank_ = 0u;

    bool is_external_ram_enabled_ = false;
    bool is_cgb_ = false;

    virtual void select_rom_bank(uint8_t data) = 0;
    virtual void select_ram_bank(uint8_t data) = 0;
    void set_external_ram_enabled(uint8_t data);
    [[nodiscard]] physical_address to_physical_address(const address16& virtual_address) const;

private:
    [[nodiscard]] virtual uint32_t get_rom_bank() const { return rom_bank_ - 1; };
    [[nodiscard]] virtual uint32_t get_ram_bank() const { return ram_bank_; };
    [[nodiscard]] uint32_t get_video_ram_bank() const;
    [[nodiscard]] uint32_t get_work_ram_bank() const;

    virtual void control(const address16& virtual_address, uint8_t data) = 0;
};

constexpr size_t operator ""_kb(const unsigned long long bytes)
{
    return bytes * 1024u;
}

}

#endif //GAMEBOY_MBC_H
