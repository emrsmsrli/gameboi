#ifndef GAMEBOY_LOG_H
#define GAMEBOY_LOG_H

#if DEBUG
#include <iostream>
#endif

#include <string_view>
#include <fmt/core.h>

namespace gameboy::log {

template<typename... Args>
void info(const std::string_view format, const Args& ... args) noexcept
{
    if constexpr(DEBUG) {
        std::cout << "[I] - " << fmt::format(format.data(), args...) << '\n';
    }
}

template<typename... Args>
void warn(const std::string_view format, const Args& ... args) noexcept
{
    if constexpr(DEBUG) {
        // fmt::print("[W] - {}\n", fmt::format(format.data(), args...));
    }
}

template<typename... Args>
[[noreturn]] void error(const std::string_view format, const Args& ... args) noexcept
{
    const auto log = fmt::format(format.data(), args...);

    if constexpr(DEBUG) {
        // fmt::print(stderr, "[E] - {}\n", log);
    }

    std::terminate();
}

} // namespace gameboy

#endif //GAMEBOY_LOG_H
