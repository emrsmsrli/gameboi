#include <thread>
#include <chrono>

#include <SFML/Graphics.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "gameboy/gameboy.h"

#if WITH_DEBUGGER
#include "debugger/debugger.h"
#endif //WITH_DEBUGGER

#include "sdl_core.h"
#include "sdl_audio.h"

namespace {

struct sfml_frontend {
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

    explicit sfml_frontend(gameboy::gameboy& gb, const uint32_t width, const uint32_t height) noexcept
        : title{fmt::format("GAMEBOY - {}", gb.rom_name())},
          window{
            sf::VideoMode(width, height),
            title,
            sf::Style::Default
          },
          audio_device{
              sdl::audio_device::device_name(0), 2u,
              sdl::audio_device::format::s16,
              gameboy::apu::sampling_rate,
              gameboy::apu::sample_size
          }
    {
#if WITH_DEBUGGER
        this->gb = gameboy::make_observer(gb);
        this->gb->get_bus()->get_cpu()->on_instruction({gameboy::connect_arg<&sfml_frontend::on_instruction>, this});
        this->gb->get_bus()->get_mmu()->on_read_access({gameboy::connect_arg<&sfml_frontend::on_read_access>, this});
        this->gb->get_bus()->get_mmu()->on_write_access({gameboy::connect_arg<&sfml_frontend::on_write_access>, this});
#endif //WITH_DEBUGGER

        window.setFramerateLimit(60u);
        window_buffer.create(gameboy::screen_width, gameboy::screen_height, sf::Color::White);
        window_texture.create(gameboy::screen_width, gameboy::screen_height);
        
        window_sprite.setTexture(window_texture);

        const auto sprite_local_bounds = window_sprite.getLocalBounds();
        window_sprite.setOrigin(sprite_local_bounds.width * .5f, sprite_local_bounds.height * .5f);

        rescale(width, height);
        render_frame();

        gb.on_render_line({gameboy::connect_arg<&sfml_frontend::render_line>, this});
        gb.on_vblank({gameboy::connect_arg<&sfml_frontend::render_frame>, this});
        gb.on_audio_buffer_full({gameboy::connect_arg<&sfml_frontend::play_sound>, this});

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

    void rescale(const uint32_t width, const uint32_t height) noexcept
    {
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
        if(debugger->has_write_access_breakpoint(addr, data)) {
            gb->tick_enabled = false;
        }
    }
#endif //WITH_DEBUGGER

    void set_framerate(sf::Time time)
    {
        window.setTitle(fmt::format("{} - FPS: {:.1f}", title, 1.f / time.asSeconds()));
    }
};

} // namespace

int main(const int argc, const char* argv[])
{
    spdlog::set_default_logger(spdlog::stdout_color_st("  core  "));

    sdl::init();

    if(argc < 2) {
        fmt::print("Usage: {} <rom_path>", argv[0]);
        return 1;
    }

    gameboy::gameboy gb{argv[1]};
    sfml_frontend frontend{gb, 600u, 600u};

#if WITH_DEBUGGER
    gameboy::debugger debugger{gb.get_bus()};
    frontend.set_debugger(gameboy::make_observer(debugger));
#endif //WITH_DEBUGGER

    sf::Clock dt;
    while(frontend.window.isOpen()) {
        sf::Event event{};
        while(frontend.window.pollEvent(event)) {
            if(event.type == sf::Event::Closed) {
                frontend.window.close();
            } else if(event.type == sf::Event::Resized) {
                const sf::FloatRect visible_area(0, 0, event.size.width, event.size.height);
                frontend.window.setView(sf::View{visible_area});
                frontend.rescale(event.size.width, event.size.height);
            } else if(event.type == sf::Event::KeyPressed) {
                switch(event.key.code) {
                    case sf::Keyboard::Up:
                        gb.press_key(gameboy::joypad::key::up);
                        break;
                    case sf::Keyboard::Down:
                        gb.press_key(gameboy::joypad::key::down);
                        break;
                    case sf::Keyboard::Left:
                        gb.press_key(gameboy::joypad::key::left);
                        break;
                    case sf::Keyboard::Right:
                        gb.press_key(gameboy::joypad::key::right);
                        break;
                    case sf::Keyboard::Z:
                        gb.press_key(gameboy::joypad::key::a);
                        break;
                    case sf::Keyboard::X:
                        gb.press_key(gameboy::joypad::key::b);
                        break;
                    case sf::Keyboard::Enter:
                        gb.press_key(gameboy::joypad::key::start);
                        break;
                    case sf::Keyboard::Space:
                        gb.press_key(gameboy::joypad::key::select);
                        break;
#if WITH_DEBUGGER
                    case sf::Keyboard::F:
                    case sf::Keyboard::F7:
                        gb.tick();
                        break;
#endif //WITH_DEBUGGER
                    default:
                        break;
                }
            } else if(event.type == sf::Event::KeyReleased) {
                switch(event.key.code) {
                    case sf::Keyboard::Up:
                        gb.release_key(gameboy::joypad::key::up);
                        break;
                    case sf::Keyboard::Down:
                        gb.release_key(gameboy::joypad::key::down);
                        break;
                    case sf::Keyboard::Left:
                        gb.release_key(gameboy::joypad::key::left);
                        break;
                    case sf::Keyboard::Right:
                        gb.release_key(gameboy::joypad::key::right);
                        break;
                    case sf::Keyboard::Z:
                        gb.release_key(gameboy::joypad::key::a);
                        break;
                    case sf::Keyboard::X:
                        gb.release_key(gameboy::joypad::key::b);
                        break;
                    case sf::Keyboard::Enter:
                        gb.release_key(gameboy::joypad::key::start);
                        break;
                    case sf::Keyboard::Space:
                        gb.release_key(gameboy::joypad::key::select);
                        break;
#if WITH_DEBUGGER
                    case sf::Keyboard::G:
                        gb.tick_one_frame();
                        break;
                    case sf::Keyboard::T:
                    case sf::Keyboard::F9:
                        gb.tick_enabled = !gb.tick_enabled;
                        break;
#endif //WITH_DEBUGGER
                    default:
                        break;
                }
            }
        }

        gb.tick_one_frame();
#if WITH_DEBUGGER
        debugger.tick();
#endif //WITH_DEBUGGER

        frontend.set_framerate(dt.restart());
    }

    sdl::quit();
    return 0;
}