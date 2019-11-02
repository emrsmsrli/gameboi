#include <SFML/Graphics.hpp>

#include <gameboy.h>

int main(int /*argc*/, char** /*argv*/)
{
    gameboy::gameboy gb("cpu_instrs.gb");
    // gb.start();

    sf::RenderWindow window(
        sf::VideoMode(160 * 4, 144 * 4),
        "GAMEBOY",
        sf::Style::Close | sf::Style::Titlebar);

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
        }
    }

    return 0;
}