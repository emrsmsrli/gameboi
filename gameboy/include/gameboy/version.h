#ifndef GAMEBOY_VERSION_H
#define GAMEBOY_VERSION_H

#include <string_view>

namespace gameboy::version {

[[maybe_unused]] constexpr auto major = 0;
[[maybe_unused]] constexpr auto minor = 6;
[[maybe_unused]] constexpr auto patch = 1;

[[maybe_unused]] constexpr std::string_view version = "0.6.1";

} // namespace gameboy::version

#endif //GAMEBOY_VERSION_H
