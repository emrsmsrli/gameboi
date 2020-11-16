#include <chrono>
#include <thread>

#include <cxxopts.hpp>
#include <fmt/core.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "frontend.h"
#include "gameboy/version.h"
#include "sdl_core.h"

#if WITH_DEBUGGER
#include "debugger/debugger.h"
#endif //WITH_DEBUGGER

int main(int argc, char* argv[])
{
    cxxopts::Options options("gameboi", "An excellent gameboy color emulator");
    options
      .show_positional_help()
      .allow_unrecognised_options()
      .add_options()
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

    if(parsed["help"].as<bool>() || parsed["rom_path"].count() == 0) {
        fmt::print(stdout, "{}", options.help());
        return 0;
    }

    spdlog::set_default_logger(spdlog::stdout_color_st("  core  "));
    spdlog::set_level(spdlog::level::from_str(parsed["verbosity"].as<std::string>()));

    sdl::init();

    const gameboy::filesystem::path rom_path = parsed["rom_path"].as<std::vector<std::string>>().front();

    frontend gb_frontend{
      parsed["width"].as<uint32_t>(),
      parsed["height"].as<uint32_t>(),
      parsed["fullscreen"].as<bool>(),
      rom_path
    };

    gameboy::gameboy gb;
    gb_frontend.register_gameboy(gameboy::make_observer(gb));

#if WITH_DEBUGGER
    gameboy::debugger debugger{gameboy::make_observer(gb)};
    gb_frontend.on_new_rom({gameboy::connect_arg<&gameboy::debugger::on_new_rom>, debugger});
#endif //WITH_DEBUGGER

    gb_frontend.window().requestFocus();
    while(true) {
        if(!gb_frontend.window().hasFocus()
#if WITH_DEBUGGER
            && !gb.tick_enabled && !debugger.has_focus()
#endif //WITH_DEBUGGER
        ) {
            sf::Event e;
            while(gb_frontend.window().pollEvent(e)) {}

            using namespace std::chrono_literals;
            std::this_thread::sleep_for(100ms);
            continue;
        }

        if(const auto tick_result = gb_frontend.tick(); tick_result == frontend::tick_result::ticking) {
            gb.tick_one_frame();
        } else if(tick_result == frontend::tick_result::should_quit) {
            break;
        }

#if WITH_DEBUGGER
        if(!gb.get_bus()->get_cartridge()->get_rom_path().empty()) {
            debugger.tick();
        }
#endif //WITH_DEBUGGER
    }

    gb.save_ram_rtc();
    sdl::quit();
    return 0;
}
