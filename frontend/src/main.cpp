#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "frontend.h"

int main(const int argc, const char* argv[])
{
    spdlog::set_default_logger(spdlog::stdout_color_st("  core  "));

    sdl::init();

    if(argc < 2) {
        fmt::print("Usage: {} <rom_path>", argv[0]);
        return 1;
    }

    gameboy::gameboy gb{argv[1]};
    frontend gb_frontend{gb, 600u, 600u};

#if WITH_DEBUGGER
    gameboy::debugger debugger{gb.get_bus()};
    gb_frontend.set_debugger(gameboy::make_observer(debugger));
#endif //WITH_DEBUGGER

    sf::Clock dt;
    while(gb_frontend.window.isOpen()) {
        sf::Event event{};
        while(gb_frontend.window.pollEvent(event)) {
            if(event.type == sf::Event::Closed) {
                gb_frontend.window.close();
            } else if(event.type == sf::Event::Resized) {
                const sf::FloatRect visible_area(0, 0, event.size.width, event.size.height);
                gb_frontend.window.setView(sf::View{visible_area});
                gb_frontend.rescale(event.size.width, event.size.height);
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

        gb_frontend.set_framerate(dt.restart());
    }

    sdl::quit();
    return 0;
}