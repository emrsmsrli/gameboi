#ifndef GAMEBOY_FRONTEND_H
#define GAMEBOY_FRONTEND_H

#include <chrono>
#include <string>
#include <thread>

#include <SFML/Graphics.hpp>
#include <fmt/format.h>

#include "gameboy/gameboy.h"
#include "sdl_audio.h"
#include "sdl_core.h"

struct frontend {
    sf::Image window_buffer;
    sf::Texture window_texture;
    sf::Sprite window_sprite;
    sf::RenderWindow window;

    sdl::audio_device audio_device;

    frontend(gameboy::gameboy& gameboy, const uint32_t width, const uint32_t height, const bool fullscreen) noexcept
      : window(
          sf::VideoMode(width, height),
          fmt::format("GAMEBOY - {}", gameboy.rom_name()),
          fullscreen ? sf::Style::Fullscreen : sf::Style::Default
        ),
        audio_device{
          sdl::audio_device::device_name(0), 2u,
          sdl::audio_device::format::s16,
          gameboy::apu::sampling_rate,
          gameboy::apu::sample_size
        }
    {
        window.setFramerateLimit(60u);
        window.setVerticalSyncEnabled(false);
        window_buffer.create(gameboy::screen_width, gameboy::screen_height, sf::Color::White);
        window_texture.create(gameboy::screen_width, gameboy::screen_height);

        window_sprite.setTexture(window_texture);

        const auto sprite_local_bounds = window_sprite.getLocalBounds();
        window_sprite.setOrigin(sprite_local_bounds.width * .5f, sprite_local_bounds.height * .5f);

        rescale_view();
        render_frame();

        gameboy.on_render_line({gameboy::connect_arg<&frontend::render_line>, this});
        gameboy.on_vblank({gameboy::connect_arg<&frontend::render_frame>, this});
        gameboy.on_audio_buffer_full({gameboy::connect_arg<&frontend::play_sound>, this});

        audio_device.resume();
    }

    void play_sound(const gameboy::apu::sound_buffer& sound_buffer) noexcept
    {
        const auto buffer_size_in_bytes = sizeof(gameboy::apu::sound_buffer::value_type) * sound_buffer.size();
        while(audio_device.queue_size() > buffer_size_in_bytes) {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1ms);
        }

        audio_device.enqueue(sound_buffer.data(), buffer_size_in_bytes);
    }

    void rescale_view() noexcept
    {
        const auto [width, height] = window.getSize();
        const auto sprite_local_bounds = window_sprite.getLocalBounds();

        const auto screen_aspect_ratio = static_cast<float>(width) / height;
        const auto sprite_aspect_ratio = sprite_local_bounds.width / sprite_local_bounds.height;
        const auto scale = screen_aspect_ratio > sprite_aspect_ratio
            ? height / sprite_local_bounds.height
            : width / sprite_local_bounds.width;

        window_sprite.setScale(scale, scale);
        window_sprite.setPosition(width * .5f, height * .5f);

        draw_sprite();
    }

    void render_line(const uint8_t line_number, const gameboy::render_line& line) noexcept
    {
        for(size_t i = 0; i < line.size(); ++i) {
            const auto& color = line[i];
            window_buffer.setPixel(i, line_number, {
                color.red, color.green, color.blue, 255
            });
        }
    }

    void draw_sprite() noexcept
    {
        window.clear();
        window.draw(window_sprite);
        window.display();
    }

    void render_frame() noexcept
    {
        window_texture.update(window_buffer);
        draw_sprite();
    }
};

#endif  // GAMEBOY_FRONTEND_H
