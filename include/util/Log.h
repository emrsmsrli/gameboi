#ifndef GAMEBOY_LOG_H
#define GAMEBOY_LOG_H

#include <iostream>
#include <string_view>
#include <fmt/core.h>

namespace gameboy::log {
    template<typename... Args>
    void info(std::string_view format, const Args& ... args)
    {
        if constexpr(DEBUG) {
            std::cout << "[I] - " << fmt::format(format.data(), args...) << '\n';
        }
    }

    template<typename... Args>
    void warn(std::string_view format, const Args& ... args)
    {
        if constexpr(DEBUG) {
            std::cout << "[W] - " << fmt::format(format.data(), args...) << '\n';
        }
    }

    template<typename... Args>
    void error(std::string_view format, const Args& ... args)
    {
        const auto log = fmt::format(format.data(), args...);

        if constexpr(DEBUG) {
            std::cout << "[E] - " << log << '\n';
        }

        throw std::runtime_error{log};
    }
}

#endif //GAMEBOY_LOG_H
