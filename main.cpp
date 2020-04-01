#include <SFML/Graphics.hpp>
#include <fmt/format.h>

#include "gameboy/gameboy.h"

#if WITH_DEBUGGER
#include "gameboy/util/observer.h"
#include "debugger/debugger.h"
#endif // WITH_DEBUGGER

namespace {

struct renderer {
    std::string title;
    sf::Image window_buffer;
    sf::Texture window_texture;
    sf::Sprite window_sprite;
    sf::RenderWindow window;

#if WITH_DEBUGGER
    gameboy::observer<gameboy::debugger> debugger;
#endif // WITH_DEBUGGER

    explicit renderer(gameboy::gameboy& gb, const uint32_t width, const uint32_t height) noexcept
        : title{fmt::format("GAMEBOY - {}", gb.rom_name())},
          window{
            sf::VideoMode(width, height),
            title,
            sf::Style::Default
          }
    {
        window.setFramerateLimit(60u);
        window_buffer.create(gameboy::screen_width, gameboy::screen_height, sf::Color::White);
        window_texture.create(gameboy::screen_width, gameboy::screen_height);
        
        window_sprite.setTexture(window_texture);

        const auto sprite_local_bounds = window_sprite.getLocalBounds();
        window_sprite.setOrigin(sprite_local_bounds.width * .5f, sprite_local_bounds.height * .5f);

        rescale(width, height);
        render_frame();

        gb.on_render_line({gameboy::connect_arg<&renderer::render_line>, this});
        gb.on_vblank({gameboy::connect_arg<&renderer::render_frame>, this});
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

#if WITH_DEBUGGER
        if(debugger) {
            debugger->tick();
        }
#endif // WITH_DEBUGGER
    }

#if WITH_DEBUGGER
    void set_debugger(const gameboy::observer<gameboy::debugger> dbgr) noexcept { debugger = dbgr; }
#endif // WITH_DEBUGGER

    void set_framerate(sf::Time time)
    {
        window.setTitle(fmt::format("{} - FPS: {:.1f}", title, 1.f / time.asSeconds()));
    }
};

} // namespace

int main(const int argc, const char* argv[])
{
    if(argc < 2) {
        fmt::print("Usage: {} <rom_path>", argv[0]);
        return 1;
    }

    gameboy::gameboy gb{argv[1]};
    renderer renderer{gb, 600u, 600u};

#if WITH_DEBUGGER
    gameboy::debugger debugger{gb.get_bus()};
    renderer.set_debugger(gameboy::make_observer(debugger));
#endif // WITH_DEBUGGER

    sf::Clock dt;
    while(renderer.window.isOpen()) {
        sf::Event event{};
        while(renderer.window.pollEvent(event)) {
            if(event.type == sf::Event::Closed) {
                renderer.window.close();
            } else if(event.type == sf::Event::Resized) {
                const sf::FloatRect visible_area(0, 0, event.size.width, event.size.height);
                renderer.window.setView(sf::View{visible_area});
                renderer.rescale(event.size.width, event.size.height);
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
#endif // WITH_DEBUGGER
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
                        debugger.gb_tick_allowed = !debugger.gb_tick_allowed;
                        break;
#endif // WITH_DEBUGGER
                    default:
                        break;
                }
            }
        }

#if WITH_DEBUGGER
        if(debugger.gb_tick_allowed) {
            gb.tick_one_frame();
        } else {
            debugger.tick();
        }
#else
        gb.tick_one_frame();
#endif // WITH_DEBUGGER

        renderer.set_framerate(dt.restart());
    }

    return 0;
}