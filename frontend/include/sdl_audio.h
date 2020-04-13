#ifndef GAMEBOY_SDL_AUDIO_H
#define GAMEBOY_SDL_AUDIO_H

#include <cstdint>
#include <string_view>

namespace sdl {

class audio_device {
public:
    enum class format {
        u8 = 0x0008,
        s8 = 0x8008,
        u16 = 0x0010,
        s16 = 0x8010,
        s32 = 0x8020,
        f32 = 0x8120
    };

    static std::string_view device_name(int32_t index);

    audio_device(std::string_view device_name,
        uint8_t channels, format format, uint32_t sampling_rate, uint16_t sample_count) noexcept;
    void resume() noexcept;
    void pause() noexcept;

    void enqueue(const void* data, size_t size_in_bytes) noexcept;
    size_t queue_size() noexcept;

private:
    uint32_t device_id_ = 0;
};

} // namespace sdl

#endif //GAMEBOY_SDL_AUDIO_H
