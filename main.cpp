#include <SFML/Graphics.hpp>
#include <fmt/format.h>

#include "gameboy/gameboy.h"
#include "debugger/debugger.h"
#include "gameboy/util/delegate.h"

namespace {

struct renderer {
    sf::Image window_buffer;
    sf::Texture window_texture;
    sf::Sprite window_sprite;
    sf::RenderWindow window;

    float scale;
    sf::Vector2f position;

    explicit renderer(gameboy::gameboy& gb, const uint32_t width, const uint32_t height)
        : window{
            sf::VideoMode(width, height),
            fmt::format("GAMEBOY - {}", gb.rom_name()),
            sf::Style::Default
        },
        scale{
            width > height
              ? height / static_cast<float>(gameboy::screen_height)
              : width / static_cast<float>(gameboy::screen_width)
        },
        position{
            (width - gameboy::screen_width * scale) * .5f,
            (height - gameboy::screen_height * scale) * .5f
        }
    {
        window.setFramerateLimit(500);
        window_buffer.create(gameboy::screen_width, gameboy::screen_height, sf::Color::White);
        window_texture.create(gameboy::screen_width, gameboy::screen_height);

        window_sprite.setTexture(window_texture);
        window_sprite.setScale(scale, scale);
        window_sprite.setPosition(position);

        render_frame();

        gb.on_render_line({gameboy::connect_arg<&renderer::render_line>, this});
        gb.on_render_frame({gameboy::connect_arg<&renderer::render_frame>, this});
    }

    void render_line(const uint8_t line_number, const gameboy::render_line& line)
    {
        for(size_t i = 0; i < line.size(); ++i) {
            const auto& color = line[i];
            window_buffer.setPixel(i, line_number, {
                color.red, color.green, color.blue, 255
            });
        }
    }

    void render_frame()
    {
        window_texture.update(window_buffer);

        window.clear();
        window.draw(window_sprite);
        window.display();
    }
};

} // namespace

int main(int argc, char* argv[])
{
    if(argc < 2) {
        fmt::print("Usage: {} <rom_path>", argv[0]);
        return 1;
    }

    gameboy::gameboy gb(argv[1]);
    renderer renderer{gb, 500u, 300u};

    gameboy::debugger debugger{gb.get_bus()};
    debugger.img = &renderer.window_buffer;

    auto tick_allowed = false;
    auto debugger_tick_allowed = false;

    struct ticker {
        bool& allowd;

        void b() { allowd = false; }
    };

    ticker t{tick_allowed};
    debugger.on_break({gameboy::connect_arg<&ticker::b>, t});

    while(renderer.window.isOpen()) {
        sf::Event event{};
        while(renderer.window.pollEvent(event)) {
            if(event.type == sf::Event::Closed) {
                renderer.window.close();
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
                    case sf::Keyboard::F:
                    case sf::Keyboard::F7:
                        gb.tick();
                        break;
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
                    case sf::Keyboard::G:
                        gb.tick_one_frame();
                        break;
                    case sf::Keyboard::T:
                    case sf::Keyboard::F9:
                        tick_allowed = !tick_allowed;
                        break;
                    case sf::Keyboard::F10:
                        debugger_tick_allowed = !debugger_tick_allowed;
                        break;
                    default:
                        break;
                }
            }
        }

        if(tick_allowed) {
            gb.tick();

            debugger.check_breakpoints();

            if(debugger_tick_allowed) {
                debugger.tick();
            }
        } else {
            debugger.tick();
        }
    }

    return 0;
}