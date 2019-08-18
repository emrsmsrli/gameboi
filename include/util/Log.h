#ifndef GAMEBOY_LOG_H
#define GAMEBOY_LOG_H

#include <fmt/core.h>
#include <iostream>

namespace gameboy::log {
    template<typename String, typename... Args>
    void info(const String& format, Args... args) {
        std::cout << fmt::format("[I] - {}\n", format, args...);
    }

    template<typename String, typename... Args>
    void warn(const String& format, Args... args) {
        std::cout << fmt::format("[W] - {}\n", format, args...);
    }

    template<typename String, typename... Args>
    void error(const String& format, Args... args) {
        std::cout << fmt::format("[E] - {}\n", format, args...);
    }
}

#endif //GAMEBOY_LOG_H
