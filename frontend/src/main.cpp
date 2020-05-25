#include <cxxopts.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "frontend.h"
#include "gameboy/version.h"

#if WITH_DEBUGGER
#include "debugger/debugger.h"
#endif //WITH_DEBUGGER

int main(int argc, char* argv[])
{
    cxxopts::Options options("gameboi", "An excellent gameboy color emulator");
    options.allow_unrecognised_options().add_options()
        ("v,version", "Print version and exit")
        ("h,help", "Show this help text")
        ("V,verbosity", "Logging verbosity", cxxopts::value<std::string>()->default_value("off"))
        ("fullscreen", "Enable fullscreen")
        ("W,width", "Width of the screen (not used if fullscreen is set)", cxxopts::value<uint32_t>()->default_value("600"))
        ("H,height", "Height of the screen (not used if fullscreen is set)", cxxopts::value<uint32_t>()->default_value("600"))
        ("rom_path", "Rom path", cxxopts::value<std::vector<std::string>>());

    options.parse_positional("rom_path");

    const auto parsed = options.parse(argc, argv);

    if(parsed["version"].as<bool>()) {
        fmt::print(stdout, "gameboi v{}", gameboy::version::version);
        return 0;
    }

    if(parsed["help"].as<bool>()) {
        fmt::print(stdout, "{}", options.help());
        return 0;
    }

    spdlog::set_default_logger(spdlog::stdout_color_st("  core  "));
    spdlog::set_level(spdlog::level::from_str(parsed["verbosity"].as<std::string>()));

    sdl::init();

    const auto rom_path = parsed["rom_path"].as<std::vector<std::string>>();

    gameboy::gameboy gb{rom_path.front()};
    frontend gb_frontend{gb,
      parsed["width"].as<uint32_t>(),
      parsed["height"].as<uint32_t>(),
      parsed["fullscreen"].as<bool>()
    };

#if WITH_DEBUGGER
    gameboy::debugger debugger{gameboy::make_observer(gb)};
#endif //WITH_DEBUGGER

    while(gb_frontend.window.isOpen()) {
        sf::Event event{};
        while(gb_frontend.window.pollEvent(event)) {
            if(event.type == sf::Event::Closed) {
                gb_frontend.window.close();
            } else if(event.type == sf::Event::Resized) {
                const sf::FloatRect visible_area(0, 0, event.size.width, event.size.height);
                gb_frontend.window.setView(sf::View{visible_area});
                gb_frontend.rescale_view();
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

        if(!gb_frontend.window.hasFocus()
#if WITH_DEBUGGER
            && !gb.tick_enabled && !debugger.has_focus()
#endif //WITH_DEBUGGER
        ) {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(50ms);
        }

        gb.tick_one_frame();
#if WITH_DEBUGGER
        debugger.tick();
#endif //WITH_DEBUGGER
    }

    sdl::quit();
    return 0;
}
