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

#if WITH_DEBUGGER
#include "debugger/debugger.h"
#endif //WITH_DEBUGGER

struct frontend {
    std::string title;
    sf::Image window_buffer;
    sf::Texture window_texture;
    sf::Sprite window_sprite;
    sf::RenderWindow window;

    sdl::audio_device audio_device;

#if WITH_DEBUGGER
    gameboy::observer<gameboy::gameboy> gb;
    gameboy::observer<gameboy::debugger> debugger;
#endif //WITH_DEBUGGER

    frontend(gameboy::gameboy& gameboy, const uint32_t width, const uint32_t height, const bool fullscreen) noexcept
      : title{fmt::format("GAMEBOY - {}", gameboy.rom_name())},
        window(
          sf::VideoMode(width, height),
          title,
          fullscreen ? sf::Style::Fullscreen : sf::Style::Default
        ),
        audio_device{
          sdl::audio_device::device_name(0), 2u,
          sdl::audio_device::format::s16,
          gameboy::apu::sampling_rate,
          gameboy::apu::sample_size
        }
    {
#if WITH_DEBUGGER
        gameboy.get_bus()->get_cpu()->on_instruction({gameboy::connect_arg<&frontend::on_instruction>, this});
        gameboy.get_bus()->get_mmu()->on_read_access({gameboy::connect_arg<&frontend::on_read_access>, this});
        gameboy.get_bus()->get_mmu()->on_write_access({gameboy::connect_arg<&frontend::on_write_access>, this});
        gb = gameboy::make_observer(gameboy);
#endif //WITH_DEBUGGER

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

#if WITH_DEBUGGER
    void set_debugger(const gameboy::observer<gameboy::debugger> dbgr) noexcept { debugger = dbgr; }

    void on_instruction(
      const gameboy::address16& addr,
      const gameboy::instruction::info& info,
      const uint16_t data) noexcept
    {
        debugger->on_instruction(addr, info, data);
        if(debugger->has_execution_breakpoint()) {
            gb->tick_enabled = false;
        }
    }

    void on_read_access(const gameboy::address16& addr) noexcept
    {
        if(debugger->has_read_access_breakpoint(addr)) {
            gb->tick_enabled = false;
        }
    }

    void on_write_access(const gameboy::address16& addr, const uint8_t data) noexcept
    {
        debugger->on_write_access(addr, data);
        if(debugger->has_write_access_breakpoint(addr, data)) {
            gb->tick_enabled = false;
        }
    }
#endif //WITH_DEBUGGER

    void set_framerate(const sf::Time& time) noexcept
    {
        window.setTitle(fmt::format("{} - FPS: {:.1f}", title, 1.f / time.asSeconds()));
    }
};

#endif  // GAMEBOY_FRONTEND_H
