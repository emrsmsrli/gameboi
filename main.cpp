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

    explicit renderer(gameboy::gameboy& gb, const uint32_t width, const uint32_t height)
        : window{
            sf::VideoMode(width, height),
            fmt::format("GAMEBOY - {}", gb.rom_name()),
            sf::Style::Default
        }
    {
        window_buffer.create(gameboy::screen_width, gameboy::screen_height, sf::Color::White);
        window_texture.create(gameboy::screen_width, gameboy::screen_height);
        
        const auto sprite_local_bounds = window_sprite.getLocalBounds();
        window_sprite.setOrigin(sprite_local_bounds.width * .5f, sprite_local_bounds.height * .5f);
        window_sprite.setTexture(window_texture);

        rescale(width, height);
        render_frame();

        gb.on_render_line({gameboy::connect_arg<&renderer::render_line>, this});
        gb.on_vblank({gameboy::connect_arg<&renderer::render_frame>, this});
    }

    void rescale(const uint32_t width, const uint32_t height)
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

    void render_line(const uint8_t line_number, const gameboy::render_line& line)
    {
        for(size_t i = 0; i < line.size(); ++i) {
            const auto& color = line[i];
            window_buffer.setPixel(i, line_number, {
                color.red, color.green, color.blue, 255
            });
        }
    }

    void draw_sprite()
    {
        window.clear();
        window.draw(window_sprite);
        window.display();
    }

    void render_frame()
    {
        window_texture.update(window_buffer);
        draw_sprite();
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
            gb.tick_one_frame();

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