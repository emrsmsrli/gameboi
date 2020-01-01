#ifndef GAMEBOY_DMA_TRANSFER_DATA_H
#define GAMEBOY_DMA_TRANSFER_DATA_H

#include <cstdint>
#include "gameboy/cpu/register8.h"
#include "gameboy/memory/address.h"
#include "gameboy/util/mathutil.h"

namespace gameboy {

struct dma_transfer_data {
    register16 source{0xFFFFu};
    register16 destination{0xFFFFu};
    register8 length_mode_start{0xFF};

    uint16_t remaining_length = 0u;

    [[nodiscard]] bool active() const noexcept { return bit_test(length_mode_start, 7u); }
    [[nodiscard]] uint16_t length() const noexcept { return ((length_mode_start & 0x7Fu) + 1) * 0x10; }

    // fixme possible bug here. active flag might be 0 instead of 1 when dma is actually active.
    void disable() noexcept { length_mode_start &= 0x7Fu; }
};

} // namespace gameboy

#endif //GAMEBOY_DMA_TRANSFER_DATA_H
