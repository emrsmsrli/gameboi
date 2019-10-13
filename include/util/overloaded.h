#ifndef GAMEBOY_OVERLOADED_H
#define GAMEBOY_OVERLOADED_H

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

#endif //GAMEBOY_OVERLOADED_H
