#ifndef GAMEBOY_INTERRUPT_REQUEST_H
#define GAMEBOY_INTERRUPT_REQUEST_H

#include <cstdint>

namespace gameboy {

struct interrupt_request {
    enum type : uint8_t {
        h_blank = 0u,
        v_blank = 1u,
        oam = 2u,
        coincidence = 3u,
    };

    uint8_t request = 0u;

    [[nodiscard]] bool none() const noexcept { return request == 0u; }
    [[nodiscard]] bool is_set(const type type) const noexcept { return bit::test(request, type); }
    void set(const type type) noexcept { request = bit::set(request, type); }
    void reset(const type type) noexcept { request = bit::reset(request, type); }
    void reset_all() noexcept { request = 0; }
};

} // namespace gameboy

#endif //GAMEBOY_INTERRUPT_REQUEST_H
