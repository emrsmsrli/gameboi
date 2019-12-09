#include <SFML/Graphics.hpp>
#include <fmt/format.h>

#include "gameboy/gameboy.h"

namespace {

constexpr auto resolution_multiplier = 1;
constexpr auto screen_width = gameboy::screen_width * resolution_multiplier;
constexpr auto screen_height = gameboy::screen_height * resolution_multiplier;

struct renderer {
    sf::Image& window_buffer;
    sf::RenderWindow& window;

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
        sf::Texture texture;
        texture.loadFromImage(window_buffer);

        sf::Sprite sprite;
        sprite.setTexture(texture);

        window.draw(sprite);
    }
};

}

int main(int /*argc*/, char** /*argv*/)
{
    gameboy::gameboy gb("cpu_instrs.gb");
    // gb.start();

    sf::Image window_buffer{};
    window_buffer.create(screen_width, screen_height);

    sf::RenderWindow window{
        sf::VideoMode(screen_width, screen_height),
        fmt::format("GAMEBOY - {}", gb.rom_name()),
        sf::Style::Close | sf::Style::Titlebar
    };

    renderer renderer{window_buffer, window};
    gb.on_render_line({gameboy::connect_arg<&renderer::render_line>, renderer});
    gb.on_render_frame({gameboy::connect_arg<&renderer::render_frame>, renderer});

    while(window.isOpen()) {
        sf::Event event{};
        while(window.pollEvent(event)) {
            if(event.type == sf::Event::Closed) {
                window.close();
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
                        gb.tick();
                    default:
                        break;
                }
            }
        }
    }

    return 0;
}