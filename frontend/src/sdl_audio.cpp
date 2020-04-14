#include <SDL2/SDL_audio.h>

#include "sdl_audio.h"
#include "sdl_macro.h"

namespace sdl {

std::string_view audio_device::device_name(const int32_t index)
{
    return SDL_GetAudioDeviceName(index, SDL_FALSE);
}

audio_device::audio_device(const std::string_view device_name,
    const uint8_t channels, const format format, const uint32_t sampling_rate, const uint16_t sample_count) noexcept
{
    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.channels = channels;
    spec.format = static_cast<SDL_AudioFormat>(format);
    spec.freq = sampling_rate;
    spec.samples = sample_count;

    device_id_ = SDL_OpenAudioDevice(device_name.data(), SDL_FALSE, &spec, nullptr, 0);
    SDL_CHECK(device_id_);
}

void audio_device::resume() noexcept
{
    SDL_PauseAudioDevice(device_id_, SDL_FALSE);
}

void audio_device::pause() noexcept
{
    SDL_PauseAudioDevice(device_id_, SDL_TRUE);
}

void audio_device::enqueue(const void* data, size_t size_in_bytes) noexcept
{
    SDL_QueueAudio(device_id_, data, size_in_bytes);
}

size_t audio_device::queue_size() noexcept
{
    return SDL_GetQueuedAudioSize(device_id_);
}

} // namespace sdl
