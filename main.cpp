#include <SFML/Graphics.hpp>
#include <fmt/format.h>

#include <gameboy.h>

namespace {

constexpr auto resolution_multiplier = 1;
constexpr auto screen_width = gameboy::screen_width * resolution_multiplier;
constexpr auto screen_height = gameboy::screen_height * resolution_multiplier;

sf::Image window_buffer;
sf::RenderWindow window;

void on_render(const uint8_t line_number, const gameboy::render_line line)
{
    // todo support resolution_multiplier other than 1
    for(size_t i = 0; i < line.size(); ++i) {
        const auto& color = line[i];
        window_buffer.setPixel(i, line_number, {
            color.red, color.green, color.blue, 255
        });
    }
}

}

int main(int /*argc*/, char** /*argv*/)
{
    gameboy::gameboy gb("cpu_instrs.gb");
    gb.on_line_render({gameboy::connect_arg<on_render>});
    // gb.start();

    window.create(sf::VideoMode(screen_width, screen_height),
        fmt::format("GAMEBOY - {}", gb.rom_name()),
        sf::Style::Close | sf::Style::Titlebar);

    window_buffer.create(screen_width, screen_height);

    while(window.isOpen()) {
        sf::Event event{};
        while(window.pollEvent(event)) {
            if(event.type == sf::Event::Closed) {
                window.close();
            } else if(event.type == sf::Event::KeyReleased) {
                if(event.key.code == sf::Keyboard::F) {
                    gb.tick();
                }
            }

            sf::Texture texture;
            texture.loadFromImage(window_buffer);

            sf::Sprite sprite;
            sprite.setTexture(texture);

            window.draw(sprite);
        }
    }

    return 0;
}