#ifndef GAMEBOY_DMA_TRANSFER_DATA_H
#define GAMEBOY_DMA_TRANSFER_DATA_H

#include <cstdint>
#include "gameboy/cpu/register8.h"
#include "gameboy/memory/address.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

struct dma_transfer_data {
    static constexpr auto unit_transfer_length = 0x10u;

    register16 source{0xFFFFu};
    register16 destination{0xFFFFu};
    register8 length_mode_start{0xFF};

    [[nodiscard]] constexpr uint16_t length() const noexcept { return ((length_mode_start & 0x7Fu) + 1) * unit_transfer_length; }
    [[nodiscard]] constexpr bool disabled() const noexcept { return bit_test(length_mode_start, 7u); }
};

} // namespace gameboy

#endif //GAMEBOY_DMA_TRANSFER_DATA_H
