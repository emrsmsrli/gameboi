#ifndef GAMEBOY_SDL_MACRO_H
#define GAMEBOY_SDL_MACRO_H

#if !DEBUG
#define SDL_CHECK(v)
#define SDL_CHECK_INIT(v) (v)
#else

#include <spdlog/spdlog.h>

#define SDL_CHECK(v) \
do { if(!v) { \
    spdlog::critical("SDL error: {}", SDL_GetError()); \
} } while(0)

#define SDL_CHECK_INIT(v) SDL_CHECK((v >= 0))
#endif //DEBUG

#endif //GAMEBOY_SDL_MACRO_H
