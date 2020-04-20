#define SDL_MAIN_HANDLED
#include "sdl_core.h"

#include <SDL.h>

#include "sdl_macro.h"

namespace sdl {

void init() noexcept
{
    SDL_CHECK_INIT(SDL_Init(SDL_INIT_AUDIO));
}

void quit() noexcept
{
    SDL_Quit();
}

} // namespace sdl
