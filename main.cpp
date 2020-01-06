#include <SFML/Graphics.hpp>
#include <fmt/format.h>

#include "gameboy/gameboy.h"
#include "debugger/debugger.h"
#include <chrono>

namespace {

constexpr auto resolution_multiplier = 1;
constexpr auto screen_width = gameboy::screen_width * resolution_multiplier;
constexpr auto screen_height = gameboy::screen_height * resolution_multiplier;

struct renderer {
    sf::Image window_buffer;
    sf::Texture window_texture;
    sf::RenderWindow window;

    explicit renderer(gameboy::gameboy& gb)
        : window{
            sf::VideoMode(500, 500),
            fmt::format("GAMEBOY - {}", gb.rom_name()),
            sf::Style::Default
        }
    {
        window_buffer.create(screen_width, screen_height);
        window_texture.create(screen_width, screen_height);
        gb.on_render_line({gameboy::connect_arg<&renderer::render_line>, this});
        gb.on_render_frame({gameboy::connect_arg<&renderer::render_frame>, this});
    }

    void render_line(const uint8_t line_number, const gameboy::render_line& line)
    {
        // todo support resolution_multiplier other than 1
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

        sf::Sprite sprite;
        sprite.setTexture(window_texture);

        window.draw(sprite);
        window.display();
    }
};

}

int main(int /*argc*/, char** /*argv*/)
{
    gameboy::gameboy gb("cpu_instrs.gb");
    // gb.start();

    renderer renderer{gb};

    gameboy::debugger debugger{gb.get_bus()};
    debugger.img = &renderer.window_buffer;

    auto tick_allowed = false;
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
                    case sf::Keyboard::F:
                    case sf::Keyboard::F7:
                        gb.tick();
                        break;
                    case sf::Keyboard::G:
                        gb.tick_one_frame();
                    case sf::Keyboard::T:
                        tick_allowed = !tick_allowed;
                        break;
                    default:
                        break;
                }
            }
        }

        if(tick_allowed) {
            gb.tick();
        }

        debugger.tick();
    }

    return 0;
}