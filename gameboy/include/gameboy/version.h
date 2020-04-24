#ifndef GAMEBOY_VERSION_H
#define GAMEBOY_VERSION_H

#include <string_view>

namespace gameboy::version {

[[maybe_unused]] constexpr auto major = 1;
[[maybe_unused]] constexpr auto minor = 2;
[[maybe_unused]] constexpr auto patch = 4;

[[maybe_unused]] constexpr std::string_view version = "1.2.4";

} // namespace gameboy::version

#endif //GAMEBOY_VERSION_H
