#ifndef GAMEBOY_VERSION_H
#define GAMEBOY_VERSION_H

#include <string_view>

namespace gameboy::version {

constexpr auto major = 0;
constexpr auto minor = 0;
constexpr auto patch = 1;

constexpr std::string_view version = "0.0.1";

} // namespace gameboy::version

#endif //GAMEBOY_VERSION_H
