#ifndef GAMEBOY_SDL_AUDIO_H
#define GAMEBOY_SDL_AUDIO_H

#include <cstdint>
#include <string_view>
#include <utility>

namespace sdl {

class audio_device {
public:
    static constexpr auto invalid_id = 0;

    enum class format {
        u8 = 0x0008,
        s8 = 0x8008,
        u16 = 0x0010,
        s16 = 0x8010,
        s32 = 0x8020,
        f32 = 0x8120
    };

    static size_t num_devices() noexcept;
    static std::string_view device_name(int32_t index) noexcept;

    audio_device(std::string_view device_name,
        uint8_t channels, format format, uint32_t sampling_rate, uint16_t sample_count) noexcept;
    ~audio_device() noexcept;

    audio_device(const audio_device&) = delete;
    audio_device& operator=(const audio_device&) = delete;

    audio_device(audio_device&& other) noexcept
        : device_id_{std::exchange(other.device_id_, invalid_id)} {}
    audio_device& operator=(audio_device&& other) noexcept;

    void resume() noexcept;
    void pause() noexcept;

    void enqueue(const void* data, size_t size_in_bytes) noexcept;
    size_t queue_size() noexcept;

private:
    uint32_t device_id_ = invalid_id;
};

} // namespace sdl

#endif //GAMEBOY_SDL_AUDIO_H
