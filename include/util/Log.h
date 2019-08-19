#ifndef GAMEBOY_LOG_H
#define GAMEBOY_LOG_H

#include <fmt/core.h>
#include <iostream>

namespace gameboy::log {
    template<typename String, typename... Args>
    void info(const String& format, Args... args) {
        std::cout << "[I] - " << fmt::format(format, args...) << '\n';
    }

    template<typename String, typename... Args>
    void warn(const String& format, Args... args) {
        std::cout << "[W] - " << fmt::format(format, args...) << '\n';
    }

    template<typename String, typename... Args>
    void error(const String& format, Args... args) {
        std::cerr << "[E] - " << fmt::format(format, args...) << '\n';
    }
}

#endif //GAMEBOY_LOG_H
