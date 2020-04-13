#ifndef GAMEBOY_SDL_MACRO_H
#define GAMEBOY_SDL_MACRO_H

#if DEBUG == 0
#define SDL_CHECK(v)
#define SDL_CHECK_INIT(v) (v)
#else
#define SDL_CHECK(v) \
do { if(!v) { \
    spdlog::critical("SDL error: {}", SDL_GetError()); \
} } while(0)

#define SDL_CHECK_INIT(v) SDL_CHECK((v >= 0))
#endif

#endif //GAMEBOY_SDL_MACRO_H
