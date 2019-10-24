#ifndef GAMEBOY_OVERLOADED_H
#define GAMEBOY_OVERLOADED_H

namespace gameboy {

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

} // namespace gameboy

#endif //GAMEBOY_OVERLOADED_H
