#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "sdl_core.h"
#include "sdl_macro.h"

namespace sdl {

void init() noexcept
{
    SDL_CHECK_INIT(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS));
}

void quit() noexcept
{
    SDL_Quit();
}

} // namespace sdl
